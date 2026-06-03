/*
BSD 2-Clause License

Copyright (c) 2017, Subrato Roy (subratoroy@hotmail.com)
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice, this
  list of conditions and the following disclaimer.

* Redistributions in binary form must reproduce the above copyright notice,
  this list of conditions and the following disclaimer in the documentation
  and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "abnf.h"
#include "dictionary.h"
#include "stringex.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct abnf_t
{
    bool is_valid;
    abnf_protcol_usage_t protocol;
    string_t* method;
    string_t* protocol_version;
    string_t* request_uri;
    string_t* status_code;
    string_t* reason_phrase;
    dictionary_t* headers;
    buffer_t* body;
} abnf_t;

static char* abnf_strndup_local(const char* src, size_t len)
{
    char* out = (char*)calloc(1, len + 1);

    if (out == NULL)
    {
        return NULL;
    }

    if (len > 0)
    {
        memcpy(out, src, len);
    }

    return out;
}

static void abnf_trim_view(const char** start, size_t* len)
{
    while (*len > 0 && isspace((unsigned char)(*start)[0]))
    {
        *start = *start + 1;
        *len = *len - 1;
    }

    while (*len > 0 && isspace((unsigned char)(*start)[*len - 1]))
    {
        *len = *len - 1;
    }
}

static bool abnf_assign_string_field(string_t* field, const char* text)
{
    if (field == NULL || text == NULL)
    {
        return false;
    }

    string_clear(field);
    return (string_append(field, text) != NULL);
}

static abnf_protcol_usage_t abnf_detect_protocol_from_version(const char* version)
{
    if (version == NULL)
    {
        return ABNF_PROTOCOL_USAGE_UNKNOWN;
    }

    if (strncmp(version, "HTTP/", 5) == 0)
    {
        return ABNF_PROTOCOL_USAGE_HTTP;
    }

    if (strncmp(version, "SIP/", 4) == 0)
    {
        return ABNF_PROTOCOL_USAGE_SIP;
    }

    return ABNF_PROTOCOL_USAGE_UNKNOWN;
}

static bool abnf_parse_start_line(abnf_t* ptr, const char* line)
{
    char* local = NULL;
    char* first = NULL;
    char* second = NULL;
    char* third = NULL;

    if (ptr == NULL || line == NULL)
    {
        return false;
    }

    local = abnf_strndup_local(line, strlen(line));

    if (local == NULL)
    {
        return false;
    }

    first = strtok(local, " ");
    second = strtok(NULL, " ");
    third = strtok(NULL, "");

    if (first == NULL || second == NULL)
    {
        free(local);
        return false;
    }

    if (strncmp(first, "HTTP/", 5) == 0 || strncmp(first, "SIP/", 4) == 0)
    {
        if (third == NULL)
        {
            free(local);
            return false;
        }

        abnf_assign_string_field(ptr->protocol_version, first);
        abnf_assign_string_field(ptr->status_code, second);
        abnf_assign_string_field(ptr->reason_phrase, third);
        ptr->protocol = abnf_detect_protocol_from_version(first);
    }
    else
    {
        if (third == NULL)
        {
            free(local);
            return false;
        }

        abnf_assign_string_field(ptr->method, first);
        abnf_assign_string_field(ptr->request_uri, second);
        abnf_assign_string_field(ptr->protocol_version, third);
        ptr->protocol = abnf_detect_protocol_from_version(third);
    }

    free(local);
    return true;
}

static bool abnf_parse_headers_block(abnf_t* ptr, const char* headers, size_t headers_len)
{
    size_t cursor = 0;

    if (ptr == NULL || headers == NULL)
    {
        return false;
    }

    while (cursor < headers_len)
    {
        const char* line = headers + cursor;
        const char* line_end = strstr(line, "\r\n");
        const char* colon = NULL;
        size_t line_len;

        if (line_end == NULL || (size_t)(line_end - headers) > headers_len)
        {
            break;
        }

        line_len = (size_t)(line_end - line);

        if (line_len == 0)
        {
            break;
        }

        colon = memchr(line, ':', line_len);

        if (colon != NULL)
        {
            const char* key = line;
            const char* value = colon + 1;
            size_t key_len = (size_t)(colon - line);
            size_t value_len = line_len - key_len - 1;
            char* key_buf = NULL;
            char* value_buf = NULL;

            abnf_trim_view(&key, &key_len);
            abnf_trim_view(&value, &value_len);

            if (key_len > 0)
            {
                key_buf = abnf_strndup_local(key, key_len);
                value_buf = abnf_strndup_local(value, value_len);

                if (key_buf == NULL || value_buf == NULL)
                {
                    free(key_buf);
                    free(value_buf);
                    return false;
                }

                dictionary_set_value(ptr->headers,
                                     key_buf,
                                     strlen(key_buf) + 1,
                                     value_buf,
                                     strlen(value_buf) + 1);

                free(key_buf);
                free(value_buf);
            }
        }

        cursor = (size_t)(line_end - headers) + 2;
    }

    return true;
}

static abnf_t* abnf_parse_internal(string_t* data, bool with_body)
{
    abnf_t* ptr = NULL;
    const char* raw = NULL;
    size_t raw_len = 0;
    char* working = NULL;
    char* header_end = NULL;
    char* first_line_end = NULL;

    if (data == NULL)
    {
        return NULL;
    }

    raw = string_c_str(data);
    raw_len = string_get_length(data);

    if (raw == NULL || raw_len == 0)
    {
        return NULL;
    }

    ptr = abnf_allocate();

    if (ptr == NULL)
    {
        return NULL;
    }

    working = abnf_strndup_local(raw, raw_len);

    if (working == NULL)
    {
        abnf_free(&ptr);
        return NULL;
    }

    first_line_end = strstr(working, "\r\n");

    if (first_line_end == NULL)
    {
        free(working);
        abnf_free(&ptr);
        return NULL;
    }

    *first_line_end = 0;

    if (!abnf_parse_start_line(ptr, working))
    {
        free(working);
        abnf_free(&ptr);
        return NULL;
    }

    header_end = strstr(first_line_end + 2, "\r\n\r\n");

    if (header_end != NULL)
    {
        size_t headers_len = (size_t)(header_end - (first_line_end + 2));

        if (!abnf_parse_headers_block(ptr, first_line_end + 2, headers_len))
        {
            free(working);
            abnf_free(&ptr);
            return NULL;
        }

        if (with_body)
        {
            const char* body_start = header_end + 4;
            size_t body_offset = (size_t)(body_start - working);

            if (body_offset <= raw_len)
            {
                size_t body_len = raw_len - body_offset;

                if (body_len > 0)
                {
                    buffer_append(ptr->body, body_start, body_len);
                }
            }
        }
    }
    else
    {
        size_t headers_len = raw_len - (size_t)((first_line_end + 2) - working);

        if (!abnf_parse_headers_block(ptr, first_line_end + 2, headers_len))
        {
            free(working);
            abnf_free(&ptr);
            return NULL;
        }
    }

    ptr->is_valid = true;
    free(working);
    return ptr;
}

abnf_t* abnf_allocate()
{
    abnf_t* ptr = (abnf_t*)calloc(1, sizeof(abnf_t));

    if (ptr == NULL)
    {
        return NULL;
    }

    ptr->is_valid = false;
    ptr->protocol = ABNF_PROTOCOL_USAGE_UNKNOWN;
    ptr->method = string_allocate_default();
    ptr->protocol_version = string_allocate_default();
    ptr->request_uri = string_allocate_default();
    ptr->status_code = string_allocate_default();
    ptr->reason_phrase = string_allocate_default();
    ptr->headers = dictionary_allocate();
    ptr->body = buffer_allocate_default();

    if (ptr->method == NULL ||
        ptr->protocol_version == NULL ||
        ptr->request_uri == NULL ||
        ptr->status_code == NULL ||
        ptr->reason_phrase == NULL ||
        ptr->headers == NULL ||
        ptr->body == NULL)
    {
        abnf_free(&ptr);
        return NULL;
    }

    return ptr;
}

abnf_t* abnf_parse_and_allocate(string_t* data)
{
    return abnf_parse_internal(data, true);
}

abnf_t* abnf_parse_and_allocate_from_headers(string_t* data)
{
    return abnf_parse_internal(data, false);
}

void abnf_free(abnf_t** ptr)
{
    if (ptr == NULL || *ptr == NULL)
    {
        return;
    }

    string_free(&(*ptr)->method);
    string_free(&(*ptr)->protocol_version);
    string_free(&(*ptr)->request_uri);
    string_free(&(*ptr)->status_code);
    string_free(&(*ptr)->reason_phrase);
    dictionary_free((*ptr)->headers);
    (*ptr)->headers = NULL;
    buffer_free(&(*ptr)->body);

    free(*ptr);
    *ptr = NULL;
}

bool abnf_is_valid(abnf_t* ptr)
{
    if (ptr == NULL)
    {
        return false;
    }

    return ptr->is_valid;
}

string_t* abnf_serialize(abnf_t* ptr, abnf_protcol_usage_t usage)
{
    string_t* result = NULL;

    if (ptr == NULL || !ptr->is_valid)
    {
        return NULL;
    }

    result = string_allocate_default();

    if (result == NULL)
    {
        return NULL;
    }

    if (usage == ABNF_PROTOCOL_USAGE_HTTP || usage == ABNF_PROTOCOL_USAGE_SIP)
    {
        if (string_get_length(ptr->method) > 0 && string_get_length(ptr->request_uri) > 0 && string_get_length(ptr->protocol_version) > 0)
        {
            string_append(result, string_c_str(ptr->method));
            string_append(result, " ");
            string_append(result, string_c_str(ptr->request_uri));
            string_append(result, " ");
            string_append(result, string_c_str(ptr->protocol_version));
            string_append(result, "\r\n");
        }
        else if (string_get_length(ptr->protocol_version) > 0 && string_get_length(ptr->status_code) > 0 && string_get_length(ptr->reason_phrase) > 0)
        {
            string_append(result, string_c_str(ptr->protocol_version));
            string_append(result, " ");
            string_append(result, string_c_str(ptr->status_code));
            string_append(result, " ");
            string_append(result, string_c_str(ptr->reason_phrase));
            string_append(result, "\r\n");
        }
        else
        {
            string_free(&result);
            return NULL;
        }
    }
    else if (usage == ABNF_PROTOCOL_USAGE_SMTP || usage == ABNF_PROTOCOL_USAGE_IMAP4)
    {
        if (string_get_length(ptr->method) > 0)
        {
            string_append(result, string_c_str(ptr->method));

            if (string_get_length(ptr->request_uri) > 0)
            {
                string_append(result, " ");
                string_append(result, string_c_str(ptr->request_uri));
            }

            string_append(result, "\r\n");
        }
        else if (string_get_length(ptr->status_code) > 0 && string_get_length(ptr->reason_phrase) > 0)
        {
            string_append(result, string_c_str(ptr->status_code));
            string_append(result, " ");
            string_append(result, string_c_str(ptr->reason_phrase));
            string_append(result, "\r\n");
        }
        else
        {
            string_free(&result);
            return NULL;
        }
    }
    else
    {
        string_free(&result);
        return NULL;
    }

    if (ptr->headers != NULL)
    {
        char** keys = dictionary_get_all_keys(ptr->headers);

        if (keys != NULL)
        {
            long idx = 0;

            while (keys[idx] != NULL)
            {
                const char* key = keys[idx];
                const char* value = (const char*)dictionary_get_value(ptr->headers, key, strlen(key) + 1);

                if (value != NULL)
                {
                    string_append(result, key);
                    string_append(result, ": ");
                    string_append(result, value);
                    string_append(result, "\r\n");
                }

                idx++;
            }

            dictionary_free_key_list(ptr->headers, keys);
        }
    }

    string_append(result, "\r\n");

    if (ptr->body != NULL && buffer_get_size(ptr->body) > 0)
    {
        string_t* body_string = buffer_convert_to_string(ptr->body);

        if (body_string != NULL)
        {
            string_append(result, string_c_str(body_string));
            string_free(&body_string);
        }
    }

    return result;
}

abnf_protcol_usage_t abnf_get_protocol(abnf_t* ptr)
{
    if (ptr == NULL)
    {
        return ABNF_PROTOCOL_USAGE_UNKNOWN;
    }

    return ptr->protocol;
}

string_t* abnf_get_protocol_version(abnf_t* ptr)
{
    if (ptr == NULL)
    {
        return NULL;
    }

    return ptr->protocol_version;
}

string_t* abnf_get_method(abnf_t* ptr)
{
    if (ptr == NULL)
    {
        return NULL;
    }

    return ptr->method;
}

string_t* abnf_get_request_uri(abnf_t* ptr)
{
    if (ptr == NULL)
    {
        return NULL;
    }

    return ptr->request_uri;
}

int abnf_get_status_code(abnf_t* ptr)
{
    if (ptr == NULL || string_get_length(ptr->status_code) == 0)
    {
        return -1;
    }

    return atoi(string_c_str(ptr->status_code));
}

string_t* abnf_get_reason_phrase(abnf_t* ptr)
{
    if (ptr == NULL)
    {
        return NULL;
    }

    return ptr->reason_phrase;
}

string_t* abnf_get_header_value(abnf_t* ptr, const char* header_name)
{
    const char* value = NULL;

    if (ptr == NULL || header_name == NULL || ptr->headers == NULL)
    {
        return NULL;
    }

    value = (const char*)dictionary_get_value(ptr->headers, header_name, strlen(header_name) + 1);

    if (value == NULL)
    {
        return NULL;
    }

    return string_allocate(value);
}

string_list_t* abnf_get_all_headers(abnf_t* ptr)
{
    char** keys = NULL;
    string_list_t* result = NULL;

    if (ptr == NULL || ptr->headers == NULL)
    {
        return NULL;
    }

    keys = dictionary_get_all_keys(ptr->headers);

    if (keys == NULL)
    {
        return NULL;
    }

    result = string_list_allocate_default();

    if (result == NULL)
    {
        dictionary_free_key_list(ptr->headers, keys);
        return NULL;
    }

    for (long i = 0; keys[i] != NULL; ++i)
    {
        string_append_to_list(result, keys[i]);
    }

    dictionary_free_key_list(ptr->headers, keys);
    return result;
}

string_t* abnf_get_body(abnf_t* ptr)
{
    if (ptr == NULL || ptr->body == NULL)
    {
        return NULL;
    }

    return buffer_convert_to_string(ptr->body);
}

bool abnf_set_body(abnf_t* ptr, const char* body)
{
    if (ptr == NULL || body == NULL)
    {
        return false;
    }

    if (ptr->body == NULL)
    {
        ptr->body = buffer_allocate_default();

        if (ptr->body == NULL)
        {
            return false;
        }
    }

    buffer_clear(ptr->body);
    buffer_append(ptr->body, body, strlen(body));
    return true;
}

bool abnf_set_header_value(abnf_t* ptr, const char* header_name, const char* header_value)
{
    if (ptr == NULL || header_name == NULL || header_value == NULL)
    {
        return false;
    }

    if (ptr->headers == NULL)
    {
        ptr->headers = dictionary_allocate();

        if (ptr->headers == NULL)
        {
            return false;
        }
    }

    dictionary_set_value(ptr->headers,
                         header_name,
                         strlen(header_name) + 1,
                         header_value,
                         strlen(header_value) + 1);

    return true;
}

bool abnf_set_method(abnf_t* ptr, const char* method)
{
    if (ptr == NULL || method == NULL || ptr->method == NULL)
    {
        return false;
    }

    return abnf_assign_string_field(ptr->method, method);
}

bool abnf_set_request_uri(abnf_t* ptr, const char* uri)
{
    if (ptr == NULL || uri == NULL || ptr->request_uri == NULL)
    {
        return false;
    }

    return abnf_assign_string_field(ptr->request_uri, uri);
}

bool abnf_set_status_code(abnf_t* ptr, int status_code)
{
    char code_str[4] = {0};

    if (ptr == NULL || status_code < 100 || status_code > 999 || ptr->status_code == NULL)
    {
        return false;
    }

    snprintf(code_str, sizeof(code_str), "%d", status_code);
    return abnf_assign_string_field(ptr->status_code, code_str);
}

bool abnf_set_reason_phrase(abnf_t* ptr, const char* reason_phrase)
{
    if (ptr == NULL || reason_phrase == NULL || ptr->reason_phrase == NULL)
    {
        return false;
    }

    return abnf_assign_string_field(ptr->reason_phrase, reason_phrase);
}

bool abnf_set_protocol_version(abnf_t* ptr, const char* version)
{
    if (ptr == NULL || version == NULL || ptr->protocol_version == NULL)
    {
        return false;
    }

    return abnf_assign_string_field(ptr->protocol_version, version);
}

bool abnf_set_protocol(abnf_t* ptr, abnf_protcol_usage_t usage)
{
    if (ptr == NULL)
    {
        return false;
    }

    ptr->protocol = usage;
    return true;
}

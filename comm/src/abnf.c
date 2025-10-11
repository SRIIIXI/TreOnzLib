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
#include "stringex.h"
#include "dictionary.h"

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
}abnf_t;

abnf_t* abnf_allocate()
{
    abnf_t* ptr = (abnf_t*)calloc(1, sizeof(abnf_t));
    if (ptr)
    {
        ptr->is_valid = false;
        ptr->protocol = ABNF_PROTOCOL_USAGE_HTTP;
        ptr->method = string_allocate();
        ptr->protocol_version = string_allocate();
        ptr->request_uri = string_allocate();
        ptr->status_code = string_allocate();
        ptr->reason_phrase = string_allocate();
        ptr->headers = dictionary_allocate();
        ptr->body = buffer_allocate();
    }
    return ptr;
}

abnf_t *abnf_parse_and_allocate(string_t *data)
{
    if (!data)
    {
        return NULL;
    }

    abnf_t* ptr = abnf_allocate();

    if (!ptr)
    {
        return NULL;
    }

    // Basic parsing logic (very simplified, real-world scenarios require more robust parsing)
    const char* raw = string_c_str(data);
    size_t len = string_get_length(data);

    // Split headers and body
    char* header_end = strstr(raw, "\r\n\r\n");
    size_t header_len = header_end ? (size_t)(header_end - raw) : len;
    size_t body_len = header_end ? (len - header_len - 4) : 0;

    // Parse start line
    char* line_end = strstr(raw, "\r\n");
    if (!line_end || (size_t)(line_end - raw) > header_len)
    {
        abnf_free(&ptr);
        return NULL;
    }

    char* start_line = strndup(raw, (size_t)(line_end - raw));
    char* rest_of_headers = line_end + 2;
    size_t rest_len = header_len - (size_t)(line_end - raw) - 2;

    // Determine protocol type based on start line format
    if (strstr(start_line, "HTTP/") == start_line || strstr(start_line, "SIP/") == start_line)
    {
        // Response line
        ptr->protocol = (strstr(start_line, "HTTP/") == start_line) ? ABNF_PROTOCOL_USAGE_HTTP : ABNF_PROTOCOL_USAGE_SIP;

        char* token = strtok(start_line, " ");
        if (token) ptr->protocol_version = string_allocate(token);
        token = strtok(NULL, " ");
        if (token) ptr->status_code = string_allocate(token);
        token = strtok(NULL, "\r\n");
        if (token) ptr->reason_phrase = string_allocate(token);
    }
    else
    {
        // Request line
        char* token = strtok(start_line, " ");
        if (token) ptr->method = string_allocate(token);
        token = strtok(NULL, " ");
        if (token) ptr->request_uri = string_allocate(token);
        token = strtok(NULL, "\r\n");
        if (token) ptr->protocol_version = string_allocate(token);

        // Determine protocol type based on method
        if (strcmp(string_c_str(ptr->method), "GET") == 0 || strcmp(string_c_str(ptr->method), "POST") == 0)
        {
            ptr->protocol = ABNF_PROTOCOL_USAGE_HTTP;
        }
        else if (strcmp(string_c_str(ptr->method), "INVITE") == 0 || strcmp(string_c_str(ptr->method), "ACK") == 0)
        {
            ptr->protocol = ABNF_PROTOCOL_USAGE_SIP;
        }
        else if (strcmp(string_c_str(ptr->method), "HELO") == 0 || strcmp(string_c_str(ptr->method), "MAIL") == 0 || strcmp(string_c_str(ptr->method), "EHLO") == 0)
        {
            ptr->protocol = ABNF_PROTOCOL_USAGE_SMTP;
        }
        else
        {
            ptr->protocol = ABNF_PROTOCOL_USAGE_UNKNOWN;
        }   
    }
    free(start_line);
    // Parse headers
    char* header_line = strtok(rest_of_headers, "\r\n");
    while (header_line && (size_t)(header_line - rest_of_headers) < rest_len)
    {
        char* colon = strchr(header_line, ':');
        if (colon)
        {
            *colon = '\0';
            char* header_name = header_line;
            char* header_value = colon + 1;
            while (*header_value == ' ') header_value++; // Trim leading spaces
            dictionary_set_value(ptr->headers, header_name, (void*)header_value, strlen(header_value) + 1);
        }
        header_line = strtok(NULL, "\r\n");
    }
    // Parse body
    if (body_len > 0 && header_end)
    {
        const char* body_start = header_end + 4;
        buffer_append(ptr->body, (const unsigned char*)body_start, body_len);
    }
    ptr->is_valid = true;
    return ptr;
}

// This function parses only the headers and does not expect a body
abnf_t *abnf_parse_and_allocate_from_headers(string_t *data)
{
    if (!data)
    {
        return NULL;
    }

    abnf_t* ptr = abnf_allocate();

    if (!ptr)
    {
        return NULL;
    }

    // Basic parsing logic (very simplified, real-world scenarios require more robust parsing)
    const char* raw = string_c_str(data);
    size_t len = string_get_length(data);

    // Parse start line
    char* line_end = strstr(raw, "\r\n");
    if (!line_end || (size_t)(line_end - raw) > len)
    {
        abnf_free(&ptr);
        return NULL;
    }

    char* start_line = strndup(raw, (size_t)(line_end - raw));
    char* rest_of_headers = line_end + 2;
    size_t rest_len = len - (size_t)(line_end - raw) - 2;

    // Determine protocol type based on start line format
    if (strstr(start_line, "HTTP/") == start_line || strstr(start_line, "SIP/") == start_line)
    {
        // Response line
        ptr->protocol = (strstr(start_line, "HTTP/") == start_line) ? ABNF_PROTOCOL_USAGE_HTTP : ABNF_PROTOCOL_USAGE_SIP;

        char* token = strtok(start_line, " ");
        if (token) ptr->protocol_version = string_allocate(token);
        token = strtok(NULL, " ");
        if (token) ptr->status_code = string_allocate(token);
        token = strtok(NULL, "\r\n");
        if (token) ptr->reason_phrase = string_allocate(token);
    }
    else
    {
        // Request line
        char* token = strtok(start_line, " ");
        if (token) ptr->method = string_allocate(token);
        token = strtok(NULL, " ");
        if (token) ptr->request_uri = string_allocate(token);
        token = strtok(NULL, "\r\n");
        if (token) ptr->protocol_version = string_allocate(token);

        // Determine protocol type based on method
        if (strcmp(string_c_str(ptr->method), "GET") == 0 || strcmp(string_c_str(ptr->method), "POST") == 0)
        {
            ptr->protocol = ABNF_PROTOCOL_USAGE_HTTP;
        }
        else if (strcmp(string_c_str(ptr->method), "INVITE") == 0 || strcmp(string_c_str(ptr->method), "ACK") == 0)
        {
            ptr->protocol = ABNF_PROTOCOL_USAGE_SIP;
        }
        else if (strcmp(string_c_str(ptr->method), "HELO") == 0 || strcmp(string_c_str(ptr->method), "MAIL") == 0 || strcmp(string_c_str(ptr->method), "EHLO") == 0)
        {
            ptr->protocol = ABNF_PROTOCOL_USAGE_SMTP;
        }
        else
        {
            ptr->protocol = ABNF_PROTOCOL_USAGE_UNKNOWN;
        }   
    }
    free(start_line);
    // Parse headers
    char* header_line = strtok(rest_of_headers, "\r\n");
    while (header_line && (size_t)(header_line - rest_of_headers) < rest_len)
    {
        char* colon = strchr(header_line, ':');
        if (colon)
        {
            *colon = '\0';
            char* header_name = header_line;
            char* header_value = colon + 1;
            while (*header_value == ' ') header_value++; // Trim leading spaces
            dictionary_set_value(ptr->headers, header_name, (void*)header_value, strlen(header_value) + 1);
        }
        header_line = strtok(NULL, "\r\n");
    }
    ptr->is_valid = true;
    return ptr;
}

void abnf_free(abnf_t** ptr)
{
    if (ptr && *ptr)
    {
        if ((*ptr)->method) string_free(&(*ptr)->method);
        if ((*ptr)->protocol_version) string_free(&(*ptr)->protocol_version);
        if ((*ptr)->request_uri) string_free(&(*ptr)->request_uri);
        if ((*ptr)->status_code) string_free(&(*ptr)->status_code);
        if ((*ptr)->reason_phrase) string_free(&(*ptr)->reason_phrase);
        if ((*ptr)->headers) dictionary_free(&(*ptr)->headers);
        if ((*ptr)->body) buffer_free(&(*ptr)->body);
        free(*ptr);
        *ptr = NULL;
    }
}   

bool abnf_is_valid(abnf_t* ptr)
{
    if (!ptr)
    {
        return false;
    }

    return ptr->is_valid;
} 

string_t* abnf_serialize(abnf_t* ptr, abnf_protcol_usage_t usage)
{
    if (!ptr)
    {
        return NULL;
    }

    if (!ptr->is_valid)
    {
        return NULL;
    }

    string_t* result = string_allocate();

    if (!result)
    {
        return NULL;
    }

    // Start line
    if (usage == ABNF_PROTOCOL_USAGE_HTTP || usage == ABNF_PROTOCOL_USAGE_SIP)
    {
        if (string_get_length(ptr->method) > 0 && string_get_length(ptr->request_uri) > 0 && string_get_length(ptr->protocol_version) > 0)
        {
            // Request line
            string_append(result, ptr->method);
            string_append(result, " ");
            string_append(result, ptr->request_uri);
            string_append(result, " ");
            string_append(result, ptr->protocol_version);
            string_append(result, "\r\n");
        }
        else if (string_get_length(ptr->protocol_version) > 0 && string_get_length(ptr->status_code) > 0 && string_get_length(ptr->reason_phrase) > 0)
        {
            // Status line
            string_append(result, ptr->protocol_version);
            string_append(result, " ");
            string_append(result, ptr->status_code);
            string_append(result, " ");
            string_append(result, ptr->reason_phrase);
            string_append(result, "\r\n");
        }
        else
        {
            // Invalid state
            string_free(&result);
            return NULL;
        }
    }
    else if (usage == ABNF_PROTOCOL_USAGE_SMTP || usage == ABNF_PROTOCOL_USAGE_IMAP4)
    {
        if (string_get_length(ptr->method) > 0)
        {
            // Command line
            string_append(result, ptr->method);
            string_append(result, "\r\n");
        }
        else if (string_get_length(ptr->status_code) > 0 && string_get_length(ptr->reason_phrase) > 0)
        {
            // Response line
            string_append(result, ptr->status_code);
            string_append(result, " ");
            string_append(result, ptr->reason_phrase);
            string_append(result, "\r\n");
        }
        else
        {
            // Invalid state
            string_free(&result);
            return NULL;
        }
    }
    else
    {
        // Unsupported protocol usage
        string_free(&result);
        return NULL;
    }   
    // Headers
    if (ptr->headers)
    {
        string_list_t* keys = dictionary_get_all_keys(ptr->headers);
        string_t* key = string_get_first_from_list(keys);
        while (key)
        {
            string_t* value = dictionary_get_value(ptr->headers, string_c_str(key));
            if (value)
            {
                string_append(result, key);
                string_append(result, ": ");
                string_append(result, value);
                string_append(result, "\r\n");
            }
            key = string_get_next_from_list(keys);
        }
        string_free_list(keys);
    }
    // End of headers
    string_append(result, "\r\n");
    // Body
    if (ptr->body && buffer_get_size(ptr->body) > 0)
    {
        string_append(result, (const char*)buffer_get_data(ptr->body));
    }
    return result;
}

abnf_protcol_usage_t abnf_get_protocol(abnf_t* ptr)
{
    if (!ptr)
    {
        return ABNF_PROTOCOL_USAGE_UNKNOWN;
    }

    return ptr->protocol;
}   

string_t* abnf_get_protocol_version(abnf_t* ptr)
{
    if (!ptr)
    {
        return NULL;
    }

    return ptr->protocol_version;
}

string_t* abnf_get_method(abnf_t* ptr)
{
    if (!ptr)
    {
        return NULL;
    }

    return ptr->method;
}

string_t* abnf_get_request_uri(abnf_t* ptr)
{
    if (!ptr)
    {
        return NULL;
    }

    return ptr->request_uri;
}

int abnf_get_status_code(abnf_t* ptr)
{
    if (!ptr)
    {
        return -1;
    }

    if (string_get_length(ptr->status_code) == 0)
    {
        return -1;
    }

    return atoi(string_c_str(ptr->status_code));
}   

string_t* abnf_get_reason_phrase(abnf_t* ptr)
{
    if (!ptr)
    {
        return NULL;
    }

    return ptr->reason_phrase;
}

string_t* abnf_get_header_value(abnf_t* ptr, const char* header_name)
{
    if (!ptr || !header_name)
    {
        return NULL;
    }

    if (!ptr->headers)
    {
        return NULL;
    }

    return dictionary_get_value(ptr->headers, header_name);
}

string_list_t* abnf_get_all_headers(abnf_t* ptr)
{
    if (!ptr)
    {
        return NULL;
    }

    if (!ptr->headers)
    {
        return NULL;
    }

    return dictionary_get_all_keys(ptr->headers);
}

string_t* abnf_get_body(abnf_t* ptr)
{
    if (!ptr)
    {
        return NULL;
    }

    if (!ptr->body)
    {
        return NULL;
    }

    string_t* bodystr = string_allocate();

    if (!bodystr)
    {
        return NULL;
    }

    size_t bodysize = buffer_get_size(ptr->body);

    if (bodysize > 0)
    {
        string_append(bodystr, (const char*)buffer_get_data(ptr->body));
    }

    return bodystr;
}

bool abnf_set_body(abnf_t* ptr, const char* body)
{
    if (!ptr || !body)
    {
        return false;
    }

    if (!ptr->body)
    {
        ptr->body = buffer_allocate();
        if (!ptr->body)
        {
            return false;
        }
    }

    buffer_clear(ptr->body);
    buffer_append(ptr->body, (const unsigned char*)body, strlen(body));
    return true;
}

bool abnf_set_header_value(abnf_t* ptr, const char* header_name, const char* header_value)
{
    if (!ptr || !header_name || !header_value)
    {
        return false;
    }

    if (!ptr->headers)
    {
        ptr->headers = dictionary_allocate();
        if (!ptr->headers)
        {
            return false;
        }
    }

    dictionary_set_value(ptr->headers, header_name, (void*)header_value, strlen(header_value) + 1);

    return true;
}   

bool abnf_set_method(abnf_t* ptr, const char* method)
{
    if (!ptr || !method)
    {
        return false;
    }

    if (!ptr->method)
    {
        ptr->method = string_allocate();
        if (!ptr->method)
        {
            return false;
        }
    }

    string_clear(ptr->method);
    string_append(ptr->method, method);
    return true;
}

bool abnf_set_request_uri(abnf_t* ptr, const char* uri)
{
    if (!ptr || !uri)
    {
        return false;
    }

    if (!ptr->request_uri)
    {
        ptr->request_uri = string_allocate();
        if (!ptr->request_uri)
        {
            return false;
        }
    }

    string_clear(ptr->request_uri);
    string_append(ptr->request_uri, uri);
    return true;
}

bool abnf_set_status_code(abnf_t* ptr, int status_code)
{
    if (!ptr || status_code < 100 || status_code > 999)
    {
        return false;
    }

    if (!ptr->status_code)
    {
        ptr->status_code = string_allocate();
        if (!ptr->status_code)
        {
            return false;
        }
    }

    char code_str[4] = { 0 };
    snprintf(code_str, sizeof(code_str), "%d", status_code);
    string_clear(ptr->status_code);
    string_append(ptr->status_code, code_str);
    return true;
}

bool abnf_set_reason_phrase(abnf_t* ptr, const char* reason_phrase)
{
    if (!ptr || !reason_phrase)
    {
        return false;
    }

    if (!ptr->reason_phrase)
    {
        ptr->reason_phrase = string_allocate();
        if (!ptr->reason_phrase)
        {
            return false;
        }
    }

    string_clear(ptr->reason_phrase);
    string_append(ptr->reason_phrase, reason_phrase);
    return true;
}

bool abnf_set_protocol_version(abnf_t* ptr, const char* version)
{
    if (!ptr || !version)
    {
        return false;
    }

    if (!ptr->protocol_version)
    {
        ptr->protocol_version = string_allocate();
        if (!ptr->protocol_version)
        {
            return false;
        }
    }

    string_clear(ptr->protocol_version);
    string_append(ptr->protocol_version, version);
    return true;
}

bool abnf_set_protocol(abnf_t* ptr, abnf_protcol_usage_t usage)
{
    if (!ptr)
    {
        return false;
    }

    ptr->protocol = usage;
    return true;
}





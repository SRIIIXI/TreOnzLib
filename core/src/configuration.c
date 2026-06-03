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

#include "configuration.h"
#include "stringex.h"
#include "environment.h"
#include "directory.h"

#include <memory.h>
#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include <float.h>
#include <string.h>

typedef struct key_value_t
{
    char* key;
    char* value;
    struct key_value_t* next;
}key_value_t;

typedef struct section_t
{
    char* section_name;
    long key_value_count;
    key_value_t* key_value_list;
    struct section_t* next;
}section_t;

typedef struct configuration_t
{
    long section_count;
    section_t* section_list;
}configuration_t;

void configuration_internal_add_section(configuration_t* conf_ptr, const char* section_name);
void configuration_internal_add_key_value(configuration_t* conf_ptr, const char* section_name, const char* key, const char* value);
const section_t* configuration_internal_get_section(const configuration_t* conf_ptr, const char *section_name);
const char* configuration_internal_get_value(const configuration_t *conf_ptr, const section_t* section, const char *key);

configuration_t* configuration_allocate_default(void)
{
    const char* user_env = getenv("USER");
    const char* home_env = getenv("HOME");
    char* filename = (char*)calloc(1024, sizeof(char));

    if (!filename)
    {
        return NULL;
    }

    if(user_env != NULL && strcmp(user_env, "root") == 0)
    {
        strcat(filename, "/etc/");
    }
    else
    {
        if (home_env == NULL)
        {
            free(filename);
            return NULL;
        }

        strcat(filename, home_env);
        strcat(filename, "/.config/");
    }

    string_t* proces_name = env_get_current_process_name();

    if (proces_name == NULL)
    {
        free(filename);
        return NULL;
    }

    strcat(filename, string_c_str(proces_name));
    strcat(filename, ".conf");
    string_free(&proces_name);

    configuration_t* ptr = configuration_allocate(filename);

    free(filename);

    return ptr;
}

configuration_t* configuration_allocate(const char* filename)
{
    if (filename == NULL)
    {
        return NULL;
    }

    configuration_t* ptr = NULL;
    FILE* fp = fopen(filename, "r");

    if(fp)
    {
        ptr = (configuration_t*)calloc(1, sizeof (configuration_t));

        if (!ptr)
        {
            fclose(fp);
            return NULL;
        }

        ptr->section_list = NULL;
        ptr->section_count = 0;

        char current_section[65] = {0};

        while(!feof(fp))
        {
            char buffer[1025] = {0};

            if(fgets(buffer, 1024, fp))
            {
                string_t* buffer_str = string_allocate(buffer);
                string_t* temp_buffer =  string_allocate(buffer);

                if (buffer_str == NULL || temp_buffer == NULL)
                {
                    string_free(&buffer_str);
                    string_free(&temp_buffer);
                    continue;
                }

                string_all_trim(temp_buffer);
                memset(buffer, 0, sizeof(buffer));
                strcpy(buffer, string_c_str(temp_buffer));
                string_free(&temp_buffer);

                if(buffer[0] == 0 || buffer[0] == ';' || buffer[0] == '#')
                {
                    string_free(&buffer_str);
                    continue;
                }

                if(buffer[0] == '[')
                {
                    string_remove_char_first(buffer_str, '[');
                    string_remove_char_first(buffer_str, ']');
                    configuration_internal_add_section(ptr, string_c_str(buffer_str));
                    memset(current_section, 0, 65);
                    strcpy(current_section, string_c_str(buffer_str));
                    string_free(&buffer_str);
                    continue;
                }

                string_t* key = NULL;
                string_t* value = NULL;

                string_split_key_value_by_char(buffer_str, '=', &key, &value);

                if (key == NULL || value == NULL)
                {
                    string_free(&key);
                    string_free(&value);
                    string_free(&buffer_str);
                    continue;
                }

                string_all_trim(key);
                string_all_trim(value);

                configuration_internal_add_key_value(ptr, current_section, string_c_str(key), string_c_str(value));
                string_free(&key);
                string_free(&value);
                string_free(&buffer_str);
            }
        }
        fclose(fp);
    }

    return ptr;
}

void  configuration_release(configuration_t* config)
{
    if(config == NULL)
    {
        return;
    }

    section_t* temp_section = NULL;
    section_t* head_section = config->section_list;

    while(head_section != NULL)
    {
        temp_section = head_section;

        key_value_t* temp_kv = NULL;
        key_value_t* head_kv = temp_section->key_value_list;

        while(head_kv != NULL)
        {
            temp_kv = head_kv;
            head_kv = head_kv->next;
            free(temp_kv->key);
            free(temp_kv->value);
            free(temp_kv);
        }

        head_section = head_section->next;
        free(temp_section->section_name);
        free(temp_section);
    }

    free(config);
}

string_list_t*  configuration_get_all_sections(const configuration_t* config)
{
    if(config == NULL)
    {
        return NULL;
    }

    string_list_t* buffer = string_list_allocate_default();

    long len = config->section_count + 1;

    if(buffer == NULL)
    {
        return NULL;
    }

    section_t* curr_section = NULL;

    for(curr_section = config->section_list; curr_section != NULL; curr_section = curr_section->next)
    {
        long temp_str_len = (long)strlen(curr_section->section_name);

        if(temp_str_len < 1)
        {
            continue;
        }

        string_append_to_list(buffer, curr_section->section_name);
    }

    return buffer;
}

string_list_t*  configuration_get_all_keys(const configuration_t *config, const char* section)
{
    if(config == NULL || section == NULL)
    {
        return NULL;
    }

    const section_t* curr_section = configuration_internal_get_section(config, section);

    if (curr_section == NULL)
    {
        return NULL;
    }

    key_value_t* curr_kv = NULL;

    string_list_t* buffer = string_list_allocate_default();

    if(buffer == NULL)
    {
        return NULL;
    }

    for(curr_kv = curr_section->key_value_list; curr_kv != NULL; curr_kv = curr_kv->next)
    {
        long temp_str_len = (long)strlen(curr_kv->value);

        if(temp_str_len < 1)
        {
            continue;
        }

        string_append_to_list(buffer, curr_kv->key);
    }

    return buffer;
}

bool  configuration_has_section(const configuration_t* config, const char* section)
{
    if(config == NULL || section == NULL)
    {
        return false;
    }

    const section_t* curr_section = configuration_internal_get_section(config, section);

    if(curr_section == NULL)
    {
        return false;
    }

    return true;
}

bool  configuration_has_key(const configuration_t* config, const char* section, char* key)
{
    if(config == NULL || section == NULL || key == NULL)
    {
        return false;
    }

    const section_t* curr_section = configuration_internal_get_section(config, section);

    if(curr_section == NULL)
    {
        return false;
    }

    const char* value = configuration_internal_get_value(config, curr_section, key);

    if(value == NULL)
    {
        return false;
    }

    return true;
}


long  configuration_get_value_as_integer(const configuration_t *config, const char* section, const char* key)
{
    if(config == NULL || section == NULL || key == NULL)
    {
        return LONG_MAX;
    }

    const section_t* curr_section = configuration_internal_get_section(config, section);

    if(curr_section == NULL)
    {
        return LONG_MAX;
    }

    const char* value = configuration_internal_get_value(config, curr_section, key);

    if(value == NULL)
    {
        return LONG_MAX;
    }

    return atol(value);
}

bool  configuration_get_value_as_boolean(const configuration_t* config, const char* section, const char* key)
{
    if(config == NULL || section == NULL || key == NULL)
    {
        return false;
    }

    const section_t* curr_section = configuration_internal_get_section(config, section);

    if(curr_section == NULL)
    {
        return false;
    }

    const char* value = configuration_internal_get_value(config, curr_section, key);

    if(value == NULL)
    {
        return false;
    }

    if(strcmp(value, "true") == 0 || strcmp(value, "1") == 0)
    {
        return true;
    }

    return false;
}

double configuration_get_value_as_real(const configuration_t *config, const char* section, const char* key)
{
    if(config == NULL || section == NULL || key == NULL)
    {
        return DBL_MAX;
    }

    const section_t* curr_section = configuration_internal_get_section(config, section);

    if(curr_section == NULL)
    {
        return DBL_MAX;
    }

    const char* value = configuration_internal_get_value(config, curr_section, key);

    if(value == NULL)
    {
        return DBL_MAX;
    }

    return atof(value);
}

const char* configuration_get_value_as_string(const configuration_t* config, const char* section, const char* key)
{
    if(config == NULL || section == NULL || key == NULL)
    {
        return NULL;
    }

    const section_t* curr_section = configuration_internal_get_section(config, section);

    if(curr_section == NULL)
    {
        return NULL;
    }

    const char* value = configuration_internal_get_value(config, curr_section, key);

    if(value == NULL)
    {
        return NULL;
    }

    return value;
}

char configuration_get_value_as_char(const configuration_t* config, const char* section, const char* key)
{
    if(config == NULL || section == NULL || key == NULL)
    {
        return 0;
    }

    const section_t* curr_section = configuration_internal_get_section(config, section);

    if(curr_section == NULL)
    {
        return 0;
    }

    const char* value = configuration_internal_get_value(config, curr_section, key);

    if(value == 0)
    {
        return 0;
    }

    return value[0];
}

void configuration_internal_add_section(configuration_t* conf_ptr, const char* section_name)
{
    if(conf_ptr == NULL)
    {
        return;
    }

    section_t* new_section = (section_t*)calloc(1, sizeof(section_t));

    if (!new_section)
    {
        return;
    }

    new_section->next = NULL;
    new_section->key_value_list = NULL;
    new_section->key_value_count = 0;
    new_section->section_name = (char*)calloc(1, strlen(section_name)+1);

    if (!new_section->section_name)
    {
        free(new_section);
        return;
    }

    strcpy(new_section->section_name, section_name);

    if(conf_ptr->section_list == NULL)
    {
        conf_ptr->section_list = new_section;
    }
    else
    {
        section_t* temp = NULL;
        for(temp = conf_ptr->section_list; temp->next != NULL; temp = temp->next) {}
        temp->next = new_section;
    }

    conf_ptr->section_count++;
}

void configuration_internal_add_key_value(configuration_t* conf_ptr, const char* section_name, const char* key, const char* value)
{
    if(conf_ptr == NULL)
    {
        return;
    }

    section_t* curr_section = NULL;

    for(curr_section = conf_ptr->section_list; curr_section != NULL; curr_section = curr_section->next)
    {
        if(strcmp(curr_section->section_name, section_name) == 0)
        {
            key_value_t* new_kv = (key_value_t*)calloc(1, sizeof (key_value_t));

            if(!new_kv)
            {
                return;
            }

            new_kv->next = NULL;
            new_kv->key = (char*)calloc(1, strlen(key)+1);

            if (!new_kv->key)
            {
                free(new_kv);
                return;
            }

            strcpy(new_kv->key, key);

            new_kv->value = (char*)calloc(1, strlen(value)+1);

            if (!new_kv->value)
            {
                free(new_kv);
                free(new_kv->key);
                return;
            }

            strcpy(new_kv->value, value);

            if(curr_section->key_value_list == NULL)
            {
                curr_section->key_value_list = new_kv;
            }
            else
            {
                key_value_t* temp = NULL;
                for(temp = curr_section->key_value_list; temp->next != NULL; temp = temp->next) {}
                temp->next = new_kv;
            }

            curr_section->key_value_count++;

            break;
        }
    }
}

const section_t *configuration_internal_get_section(const configuration_t *conf_ptr, const char* section_name)
{
    if(conf_ptr == NULL)
    {
        return NULL;
    }

    section_t* curr_section = NULL;

    for(curr_section = conf_ptr->section_list; curr_section != NULL; curr_section = curr_section->next)
    {
        if(strcmp(curr_section->section_name, section_name) == 0)
        {
            return curr_section;
        }
    }

    return NULL;
}

const char *configuration_internal_get_value(const configuration_t* conf_ptr, const section_t *section, const char* key)
{
    if(conf_ptr == NULL || section == NULL || key == NULL)
    {
        return NULL;
    }

    key_value_t* curr_kv = NULL;

    for(curr_kv = section->key_value_list; curr_kv != NULL; curr_kv = curr_kv->next)
    {
        if(strcmp(curr_kv->key, key) == 0)
        {
            return curr_kv->value;
        }
    }

    return NULL;
}

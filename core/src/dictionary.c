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

#include "dictionary.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined (_WIN32) || defined (_WIN64)
#else
#include <unistd.h>
#endif

typedef struct key_value_t
{
    void* key;
    void* value;
    bool is_value;
    size_t value_size;
    size_t key_size;
    struct key_value_t* next;
}key_value_t;

typedef struct hash_bucket_t
{
    unsigned long hash;
    long key_value_count;
    key_value_t* key_value_list;
    struct hash_bucket_t* next;
}hash_bucket_t;

typedef struct dictionary_t
{
    long hash_count;
    hash_bucket_t* hash_bucket;
}dictionary_t;

void dictionary_internal_add_hash_bucket(dictionary_t* dict_ptr, const void* key, const size_t key_size);
void dictionary_internal_add_key_value(dictionary_t* dict_ptr, const unsigned long hash, const void *key, size_t key_size, const void* value, const size_t value_size);
void dictionary_internal_add_key_reference(dictionary_t* dict_ptr, const unsigned long hash, const void *key, size_t key_size, const void* reference);
unsigned long dictionary_internal_get_hash(const void* key, const size_t key_size);

dictionary_t* dictionary_allocate()
{
    dictionary_t* ptr = (dictionary_t*)calloc(1, sizeof (dictionary_t));

    if (ptr == NULL)
    {
        return NULL;
    }

    ptr->hash_bucket = NULL;
    ptr->hash_count = 0;
    return ptr;
}

void dictionary_free(dictionary_t* dict_ptr)
{
    if(dict_ptr == NULL)
    {
        return;
    }

    hash_bucket_t* temp_hash_bucket = NULL;
    hash_bucket_t* head_hash_bucket = dict_ptr->hash_bucket;

    while(head_hash_bucket != NULL)
    {
        temp_hash_bucket = head_hash_bucket;

        key_value_t* temp_kv = NULL;
        key_value_t* head_kv = temp_hash_bucket->key_value_list;

        while(head_kv != NULL)
        {
            temp_kv = head_kv;
            head_kv = head_kv->next;
            free(temp_kv->key);
            if(temp_kv->is_value)
            {
                free(temp_kv->value);
            }
            free(temp_kv);
        }

        head_hash_bucket = head_hash_bucket->next;
        free(temp_hash_bucket);

    }

    free(dict_ptr);
}

void dictionary_set_value(dictionary_t *dict_ptr, const void* key, const size_t key_size, const void* value, const size_t value_size)
{
    if(dict_ptr == NULL || key == NULL || key_size == 0 || value == NULL || value_size == 0)
    {
        return;
    }

    hash_bucket_t* current_hash_bucket = dict_ptr->hash_bucket;

    while(current_hash_bucket != NULL)
    {
        key_value_t* current_kv = current_hash_bucket->key_value_list;

        while(current_kv)
        {
            if(current_kv->key_size == key_size && memcmp(current_kv->key, key, key_size) == 0)
            {
                if (current_kv->is_value)
                {
                    free(current_kv->value);
                }

                current_kv->value = calloc(1, value_size);

                if (current_kv->value == NULL)
                {
                    current_kv->is_value = false;
                    current_kv->value_size = 0;
                    return;
                }

                current_kv->is_value = true;
                current_kv->value_size = value_size;
                memcpy(current_kv->value, value, value_size);
                return;
            }

            current_kv = current_kv->next;
        }

        current_hash_bucket = current_hash_bucket->next;
    }

    unsigned long current_hash = dictionary_internal_get_hash(key, key_size);
    dictionary_internal_add_hash_bucket(dict_ptr, key, key_size);
    dictionary_internal_add_key_value(dict_ptr, current_hash, key, key_size, value, value_size);
}

void dictionary_set_reference(dictionary_t* dict_ptr, const void* key, const size_t key_size, const void* reference)
{
    if(dict_ptr == NULL || key == NULL || key_size == 0 || reference == NULL)
    {
        return;
    }

    hash_bucket_t* current_hash_bucket = dict_ptr->hash_bucket;

    while(current_hash_bucket != NULL)
    {
        key_value_t* current_kv = current_hash_bucket->key_value_list;

        while(current_kv)
        {
            if(current_kv->key_size == key_size && memcmp(current_kv->key, key, key_size) == 0)
            {
                if (current_kv->is_value)
                {
                    free(current_kv->value);
                }

                current_kv->value = (void*)reference;
                current_kv->is_value = false;
                current_kv->value_size = 0;
                return;
            }

            current_kv = current_kv->next;
        }

        current_hash_bucket = current_hash_bucket->next;
    }

    unsigned long current_hash = dictionary_internal_get_hash(key, key_size);
    dictionary_internal_add_hash_bucket(dict_ptr, key, key_size);
    dictionary_internal_add_key_reference(dict_ptr, current_hash, key, key_size, reference);
}

void* dictionary_get_value(dictionary_t* dict_ptr, const void *key, const size_t key_size)
{
    if(dict_ptr == NULL || key == NULL || key_size == 0)
    {
        return NULL;
    }

    hash_bucket_t* current_hash_bucket = dict_ptr->hash_bucket;

    while(current_hash_bucket != NULL)
    {
        key_value_t* current_kv = current_hash_bucket->key_value_list;

        while(current_kv)
        {
            if(current_kv->key_size == key_size && memcmp(current_kv->key, key, key_size) == 0)
            {
                return current_kv->value;
            }

            current_kv = current_kv->next;
        }

        current_hash_bucket = current_hash_bucket->next;
    }

    return NULL;
}

char** dictionary_get_all_keys(dictionary_t* dict_ptr)
{
    if(dict_ptr == NULL)
    {
        return NULL;
    }

    long total_keys = 0;
    hash_bucket_t* current_hash_bucket = NULL;
    long index = 0;

    current_hash_bucket = dict_ptr->hash_bucket;
    while( current_hash_bucket != NULL)
    {
        total_keys = total_keys + current_hash_bucket->key_value_count;
        current_hash_bucket = current_hash_bucket->next;
    }

    char** buffer = NULL;
    buffer = (char **)calloc(1, (unsigned long)(total_keys + 1) * sizeof(char*));

    current_hash_bucket = dict_ptr->hash_bucket;
    while( current_hash_bucket != NULL)
    {
        key_value_t* current_kv = current_hash_bucket->key_value_list;

        while(current_kv)
        {
            buffer[index] = (char*)calloc(current_kv->key_size+1, sizeof(char));
            memcpy(buffer[index], current_kv->key, current_kv->key_size);
            current_kv = current_kv->next;
            index++;
        }
        current_hash_bucket = current_hash_bucket->next;
    }

    return buffer;
}

void dictionary_free_key_list(dictionary_t* dict_ptr, char **key_list)
{
    if(dict_ptr == NULL || key_list == NULL)
    {
        return;
    }

    long index = 0;

    while(key_list[index] != NULL)
    {
        free(key_list[index]);
        index++;
    }

    free(key_list);
}

void dictionary_internal_add_hash_bucket(dictionary_t *dict_ptr, const void *key, const size_t key_size)
{
    if(dict_ptr == NULL)
    {
        return;
    }

    hash_bucket_t* new_hash_bucket = (hash_bucket_t*)calloc(1, sizeof(hash_bucket_t));
    new_hash_bucket->next = NULL;
    new_hash_bucket->key_value_list = NULL;
    new_hash_bucket->key_value_count = 0;
    new_hash_bucket->hash = dictionary_internal_get_hash(key, key_size);

    if(dict_ptr->hash_bucket == NULL)
    {
        dict_ptr->hash_bucket = new_hash_bucket;
    }
    else
    {
        hash_bucket_t* temp = NULL;
        for(temp = dict_ptr->hash_bucket; temp->next != NULL; temp = temp->next) {}
        temp->next = new_hash_bucket;
    }

    dict_ptr->hash_count++;
}

void dictionary_internal_add_key_value(dictionary_t *dict_ptr, const unsigned long hash, const void* key, size_t key_size, const void *value, const size_t value_size)
{
    if(dict_ptr == NULL)
    {
        return;
    }

    hash_bucket_t* curr_hash_bucket = NULL;

    for(curr_hash_bucket = dict_ptr->hash_bucket; curr_hash_bucket != NULL; curr_hash_bucket = curr_hash_bucket->next)
    {
        if(curr_hash_bucket->hash ==  hash)
        {
            key_value_t* new_kv = (key_value_t*)calloc(1, sizeof (key_value_t));
            new_kv->is_value = true;
            new_kv->next = NULL;
            new_kv->key = (char*)calloc(1, key_size);
            memcpy(new_kv->key, key, key_size);
            new_kv->value = (char*)calloc(1, value_size);
            memcpy(new_kv->value, value, value_size);
            new_kv->value_size = value_size;
            new_kv->key_size = key_size;

            if(curr_hash_bucket->key_value_list == NULL)
            {
                curr_hash_bucket->key_value_list = new_kv;
            }
            else
            {
                key_value_t* temp = NULL;
                for(temp = curr_hash_bucket->key_value_list; temp->next != NULL; temp = temp->next) {}
                temp->next = new_kv;
            }

            curr_hash_bucket->key_value_count++;

            break;
        }
    }
}

void dictionary_internal_add_key_reference(dictionary_t* dict_ptr, const unsigned long hash, const void *key, size_t key_size, const void* reference)
{
    if(dict_ptr == NULL)
    {
        return;
    }

    hash_bucket_t* curr_hash_bucket = NULL;

    for(curr_hash_bucket = dict_ptr->hash_bucket; curr_hash_bucket != NULL; curr_hash_bucket = curr_hash_bucket->next)
    {
        if(curr_hash_bucket->hash ==  hash)
        {
            key_value_t* new_kv = (key_value_t*)calloc(1, sizeof (key_value_t));
            new_kv->is_value = false;
            new_kv->next = NULL;
            new_kv->key = (char*)calloc(1, key_size);
            memcpy(new_kv->key, key, key_size);
            new_kv->value = (void*)reference;
            new_kv->key_size = key_size;

            if(curr_hash_bucket->key_value_list == NULL)
            {
                curr_hash_bucket->key_value_list = new_kv;
            }
            else
            {
                key_value_t* temp = NULL;
                for(temp = curr_hash_bucket->key_value_list; temp->next != NULL; temp = temp->next) {}
                temp->next = new_kv;
            }

            curr_hash_bucket->key_value_count++;

            break;
        }
    }
}

unsigned long dictionary_internal_get_hash(const void *key, const size_t key_size)
{
    unsigned long hash = 0;
    size_t index = 0;

    if (!key)
        return 0;

    const char* key_buffer = (const char*)key;

    for(hash = 0, index = 0 ; index < key_size ; index++)
    {
        hash += (unsigned)key_buffer[index] ;
        hash += (hash<<10);
        hash ^= (hash>>6) ;
    }

    hash += (hash <<3);
    hash ^= (hash >>11);
    hash += (hash <<15);

    return hash;
}

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

#include "buffer.h"
#include "stringex.h"

#include <string.h>
#include <memory.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <stdint.h>

typedef struct buffer_t
{
    char* data;
    size_t data_size;
    size_t memory_size;
}buffer_t;

buffer_t* buffer_internal_adjust_storage(buffer_t* buffer_ptr, size_t sz);
static size_t buffer_internal_page_size(void);
static size_t buffer_internal_initial_capacity(size_t sz);

static size_t buffer_internal_page_size(void)
{
    long psize = sysconf(_SC_PAGESIZE);

    if (psize <= 0)
    {
        return 4096;
    }

    return (size_t)psize;
}

static size_t buffer_internal_initial_capacity(size_t sz)
{
    size_t page = buffer_internal_page_size();

    if (sz == 0)
    {
        return page;
    }

    if (sz > page)
    {
        return sz;
    }

    return page;
}

buffer_t* buffer_allocate(const void *data, size_t sz)
{
    if (data == NULL && sz > 0)
    {
        return NULL;
    }

    buffer_t* nd = (buffer_t*)calloc(1, sizeof(buffer_t));

    if(nd != NULL)
    {
        nd->memory_size = buffer_internal_initial_capacity(sz);
        nd->data_size = sz;
        nd->data = (char*)calloc(nd->memory_size, sizeof (char));

        if(nd->data != NULL && data != NULL && sz > 0)
        {
            memcpy(nd->data, data, sz);
        }
    }
    return nd;
}

buffer_t* buffer_allocate_default(void)
{
    buffer_t* nd = (buffer_t*)calloc(1, sizeof(buffer_t));

    if(nd != NULL)
    {
        nd->memory_size = buffer_internal_page_size();
        nd->data_size = 0;
        nd->data = (char*)calloc(nd->memory_size, sizeof (char));
    }
    return nd;
}

buffer_t* buffer_allocate_length(size_t len)
{
    buffer_t* nd = (buffer_t*)calloc(1, sizeof(buffer_t));

    if(nd != NULL)
    {
        nd->memory_size = len > 0 ? len : 1;
        nd->data_size = 0;
        nd->data = (char*)calloc(nd->memory_size, sizeof (char));
    }
    return nd;
}

buffer_t* buffer_copy(buffer_t* dest, buffer_t* orig)
{
    if(orig != NULL && dest != NULL)
    {
        if(orig->data != NULL)
        {
            if(dest->data != NULL)
            {
                free(dest->data);
                dest->data = NULL;
                dest->data_size = 0;
            }

            dest->memory_size = orig->memory_size > 0 ? orig->memory_size : 1;
            dest->data = (char*)calloc(1, dest->memory_size);
            if (dest->data)
            {
                memcpy(dest->data, orig->data, orig->data_size);
                dest->data_size = orig->data_size;
            }
        }
    }

    return  dest;
}

buffer_t *buffer_append(buffer_t* dest, const void *data, size_t sz)
{
    if(data == NULL || sz < 1)
    {
        return NULL;
    }

    if(dest == NULL)
    {
        dest = buffer_allocate(data, sz);
        return dest;
    }
    else
    {
        dest = buffer_internal_adjust_storage(dest, sz);

        if (dest == NULL || dest->data == NULL || (dest->memory_size - dest->data_size) < sz)
        {
            return NULL;
        }

        memcpy(&dest->data[dest->data_size], data, sz);
        dest->data_size = dest->data_size + sz;
    }

    return dest;
}

void buffer_remove(buffer_t* ptr, size_t start, size_t len)
{
    if(ptr == NULL || ptr->data == NULL || len < 1)
    {
        return;
    }

    if(start > ptr->data_size || len > (ptr->data_size - start))
    {
        return;
    }

    size_t old_size = ptr->data_size;
    size_t move_bytes = old_size - (start + len);

    if (move_bytes > 0)
    {
        memmove(ptr->data + start, ptr->data + start + len, move_bytes);
    }

    memset(ptr->data + old_size - len, 0, len);
    ptr->data_size = old_size - len;
}

void buffer_remove_end(buffer_t* ptr, size_t len)
{
    if(ptr == NULL || ptr->data == NULL || len < 1 || len > ptr->data_size)
    {
        return;
    }

    size_t pos = ptr->data_size - len;
    memset(ptr->data + pos, 0, len);

    ptr->data_size = ptr->data_size - len;

    return;
}

void buffer_remove_start(buffer_t* ptr, size_t len)
{
    buffer_remove(ptr, 0, len);
}

void buffer_free(buffer_t** ptr)
{
    if(ptr == NULL || *ptr == NULL)
    {
        return;
    }

    free((*ptr)->data);
    (*ptr)->data = NULL;
    free(*ptr);
    *ptr = NULL; 
}

void buffer_clear(buffer_t* ptr)
{
    if(ptr == NULL)
    {
        return;
    }

    if(ptr->data)
    {
        for(size_t i = 0; i <  ptr->data_size; ++i)
          ptr->data[i] = 0;
    }
    ptr->data_size = 0;
}

bool buffer_is_equal(buffer_t* first, buffer_t* second)
{
    if(first != NULL && second != NULL)
    {
       if (first->data != NULL && second->data != NULL)
        {
            if(first->data_size != second->data_size)
            {
                return false;
            }

            if(memcmp(first->data, second->data, first->data_size) == 0)
            {
                return true;
            }
        }
    }

    return false;
}

bool buffer_is_greater(buffer_t* first, buffer_t* second)
{
    if(first != NULL && second != NULL)
    {
        if(first->data != NULL && second->data != NULL)
        {
            if(first->data_size != second->data_size)
            {
                return false;
            }

            if(memcmp(first->data, second->data, first->data_size) > 0)
            {
                return true;
            }
        }
    }

    return false;
}

bool buffer_is_less(buffer_t* first, buffer_t* second)
{
    if(first != NULL && second != NULL)
    {
        if(first->data != NULL && second->data != NULL)
        {
            if(first->data_size != second->data_size)
            {
                return false;
            }

            if(memcmp(first->data, second->data, first->data_size) < 0)
            {
                return true;
            }
        }
    }

    return false;
}

bool buffer_is_null(buffer_t* ptr)
{
    if(ptr == NULL)
    {
        return true;
    }
    else
    {
        if(ptr->data == NULL)
        {
            return true;
        }
        else
        {
            return false;
        }
    }

    return false;
}

const void *buffer_get_data(const buffer_t* ptr)
{
    if(ptr == NULL)
    {
        return NULL;
    }

    return ptr->data;
}

size_t buffer_get_size(const buffer_t* ptr)
{
    if(ptr == NULL)
    {
        return 0;
    }

    return ptr->data_size;
}

buffer_t* buffer_internal_adjust_storage(buffer_t* buffer_ptr, size_t sz)
{
    if(buffer_ptr == NULL)
    {
        return NULL;
    }

    if (buffer_ptr->data == NULL)
    {
        return NULL;
    }

    if (sz <= (buffer_ptr->memory_size - buffer_ptr->data_size))
    {
        return buffer_ptr;
    }

    size_t new_size = buffer_ptr->memory_size;

    while ((new_size - buffer_ptr->data_size) < sz)
    {
        if (new_size > (SIZE_MAX / 2))
        {
            return buffer_ptr;
        }

        new_size = new_size * 2;
    }

    void* ptr = (char*)calloc(new_size, sizeof (char));

    if (ptr)
    {
        memcpy(ptr, buffer_ptr->data, buffer_ptr->data_size);
        free(buffer_ptr->data);
        buffer_ptr->data = ptr;
        buffer_ptr->memory_size = new_size;
    }

    return  buffer_ptr;
}

string_t* buffer_convert_to_string(buffer_t* ptr)
{
    if(ptr == NULL)
    {
        return NULL;
    }

    //The caller must be absolutely sure that the buffer contains a string

    char* temp = (char*)calloc(ptr->data_size + 1, sizeof(char));

    if (temp == NULL)
    {
        return NULL;
    }

    if (ptr->data_size > 0)
    {
        memcpy(temp, ptr->data, ptr->data_size);
    }

    string_t* resp = string_allocate(temp);
    free(temp);

    return resp;
}

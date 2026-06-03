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

#include "stack.h"
#include "list.h"

#include <string.h>
#include <memory.h>
#include <stdlib.h>
#include <limits.h>

typedef struct stack_t
{
    list_t* list;
}stack_t;

stack_t* stack_allocate(stack_t* sptr)
{
    sptr = (stack_t*)calloc(1, sizeof(stack_t));

    if (sptr)
    {
        sptr->list = list_allocate(sptr->list);

        if (sptr->list == NULL)
        {
            free(sptr);
            return NULL;
        }
    }

    return sptr;
}

void stack_clear(stack_t* sptr)
{
    if (sptr == NULL)
    {
        return;
    }

    if (sptr->list == NULL)
    {
        return;
    }

    list_clear(sptr->list);
}

void stack_free(stack_t* sptr)
{
    if (sptr == NULL)
    {
        return;
    }

    if (sptr->list != NULL)
    {
        list_free(sptr->list);
    }

    free(sptr);
}

void stack_push(stack_t* sptr, void* data, size_t sz)
{
    if (sptr == NULL)
    {
        return;
    }

    if (sptr->list == NULL)
    {
        return;
    }

    list_add_to_tail(sptr->list, data, sz);
}

void* stack_pop(stack_t* sptr, size_t* out_size)
{
    if (out_size != NULL)
    {
        *out_size = 0;
    }

    if (sptr == NULL)
    {
        return NULL;
    }

    if (sptr->list == NULL)
    {
        return NULL;
    }

    void* ptr = NULL;
    void* out_ptr = NULL;

    size_t size = 0;
    ptr = list_get_last(sptr->list, &size);
    if (ptr == NULL)
    {
        return NULL;
    }

    out_ptr = calloc(1, size);
    if (out_ptr == NULL)
    {
        return NULL;
    }

    memcpy(out_ptr, ptr, size);
    list_remove_from_tail(sptr->list);

    if (out_size != NULL)
    {
        *out_size = size;
    }
    return out_ptr;
}

long stack_item_count(stack_t* sptr)
{
    if (sptr != NULL)
    {
        if (sptr->list != NULL)
        {
            return list_item_count(sptr->list);
        }
    }

    return -1;
}

void* stack_peek(stack_t* sptr, size_t* out_size)
{
    if (out_size != NULL)
    {
        *out_size = 0;
    }

    if (sptr == NULL)
    {
        return NULL;
    }

    if (sptr->list == NULL)
    {
        return NULL;
    }

    void* ptr = NULL;

    size_t size = 0;
    ptr = list_get_last(sptr->list, &size);
    if (ptr == NULL)
    {
        return NULL;
    }

    if (out_size != NULL)
    {
        *out_size = size;
    }
    return ptr;
}

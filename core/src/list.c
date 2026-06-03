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

#include "list.h"

#include <string.h>
#include <memory.h>
#include <stdlib.h>
#include <limits.h>

typedef int (*list_cmp_fn)(const void* dataA, size_t sizeA, const void* dataB, size_t sizeB);

typedef struct node_t
{
    void* data;
    size_t size;
    struct node_t* next;
}node_t;

typedef struct list_t
{
    long count;
    node_t* head;
    node_t* tail;
    node_t* iterator;
}list_t;

void list_internal_remove_from_head(list_t* lptr);
void list_internal_remove_from_tail(list_t* lptr);
void list_internal_add_to_head(list_t* lptr, node_t *ptr);
void list_internal_add_to_tail(list_t* lptr, node_t* ptr);
void list_internal_copy_nodes_to_list(list_t* dest, list_t* src);

static void list_internal_split(node_t* source, node_t** frontRef, node_t** backRef);
static node_t* list_internal_sorted_merge(node_t* a, node_t* b, list_cmp_fn cmp);
static node_t* list_internal_merge_sort_nodes(node_t* head, list_cmp_fn cmp);
static int list_internal_default_node_cmp(const void* dataA, size_t sizeA, const void* dataB, size_t sizeB);

list_t * list_allocate(list_t* lptr)
{
    lptr = (list_t*)calloc(1, sizeof(list_t));

    if (!lptr)
    {
        return NULL;
    }

    lptr->count = 0;
    lptr->head = lptr->tail = NULL;
    lptr->iterator = NULL;
    return lptr;
}

void list_clear(list_t* lptr)
{
    if(lptr == NULL)
    {
        return;
    }
    else
    {
        while(lptr->count > 0)
        {
            list_internal_remove_from_tail(lptr);
        }
    }
}

void list_free(list_t* lptr)
{
    if(lptr == NULL)
    {
        return;
    }
    else
    {
        while(lptr->count > 0)
        {
            list_internal_remove_from_tail(lptr);
        }

        free(lptr);
    }
}

void list_add_to_head(list_t* lptr, void* data, size_t sz)
{
    list_insert(lptr, data, sz, 0);
}

void list_add_to_tail(list_t* lptr, void* data, size_t sz)
{
    list_insert(lptr, data, sz, LONG_MAX);
}

void list_insert(list_t* lptr, void* data, size_t sz, long pos)
{
    if (lptr == NULL || data == NULL || sz == 0)
    {
        return;
    }

    if (pos == LONG_MAX)
    {
        pos = lptr->count;
    }

    // Bounds check: pos must be between 0 and lptr->count (inclusive)
    if (pos < 0 || pos > lptr->count)
    {
        return;
    }

    node_t* ptr = NULL;
    ptr = (node_t*)calloc(1, sizeof(node_t));

    if (ptr == NULL)
    {
        return;
    }

    ptr->data = calloc(1, sz);
    if (ptr->data == NULL)
    {
        free(ptr);
        return;
    }

    memcpy(ptr->data, data, sz);
    ptr->size = sz;

    if (pos == 0)
    {
        list_internal_add_to_head(lptr, ptr);
        return;
    }

    if (pos == lptr->count)
    {
        list_internal_add_to_tail(lptr, ptr);
        return;
    }

    size_t idx = 1;
    for (node_t* curptr = lptr->head; curptr->next != NULL; curptr = curptr->next, idx++)
    {
        if (pos == idx)
        {
            node_t* oldnext = curptr->next;
            curptr->next = ptr;
            ptr->next = oldnext;
            lptr->count++;
            break;
        }
    }
}

void list_remove_from_head(list_t* lptr)
{
    list_remove_at(lptr, 0);
}

void list_remove_from_tail(list_t* lptr)
{
    list_remove_at(lptr, LONG_MAX);
}

void list_remove_at(list_t* lptr, long pos)
{
    if (lptr == NULL || pos < 0)
    {
        return;
    }

    if (pos == LONG_MAX)
    {
        pos = lptr->count - 1;
    }

    if (pos > lptr->count - 1)
    {
        return;
    }

    if (pos == 0)
    {
        list_internal_remove_from_head(lptr);
        return;
    }

    if (pos == lptr->count - 1)
    {
        list_internal_remove_from_tail(lptr);
        return;
    }

    node_t *prev = NULL;
    node_t *cur = lptr->head;

    for (size_t idx = 0; idx < pos; idx++)
    {
        prev = cur;
        cur = cur->next;
    }

    if (prev != NULL && cur != NULL)
    {
        prev->next = cur->next;
        free(cur->data);
        free(cur);
        lptr->count--;
    }
}

void list_remove_value(list_t* lptr, void* data, size_t sz)
{
    if (lptr == NULL || data == NULL)
    {
        return;
    }

    node_t* ptr = NULL;
    ptr = lptr->head;
    node_t* prev = NULL;
    size_t idx = 0;

    while (ptr != NULL)
    {
        if (ptr->size == sz && memcmp(ptr->data, data, sz) == 0)
        {
            if (idx == 0)
            {
                list_internal_remove_from_head(lptr);
            }
            else if (idx >= lptr->count - 1)
            {
                list_internal_remove_from_tail(lptr);
            }
            else
            {
                prev->next = ptr->next;
                free(ptr->data);
                free(ptr);
                lptr->count--;
            }
            break;
        }

        prev = ptr;
        ptr = ptr->next;
        idx++;
    }
}

long list_item_count(list_t* lptr)
{
    if(lptr != NULL)
    {
        return lptr->count;
    }

    return 0;
}

long list_index_of(list_t *lptr, const void *node)
{
    if (lptr == NULL)
    {
        return -1;
    }

    node_t* ptr = NULL;
    ptr = lptr->head;
    size_t idx = 0;

    while (ptr != NULL)
    {
        if (ptr->data == node)
        {
            return idx;
        }

        ptr = ptr->next;
        idx++;
    }

    return -1;
}

long list_index_of_value(list_t* lptr, void* data, size_t sz)
{
    if (lptr == NULL || data == NULL || sz == 0)
    {
        return -1;
    }

    node_t* ptr = NULL;
    ptr = lptr->head;
    size_t idx = 0;

    while (ptr != NULL)
    {
        if (ptr->size == sz && memcmp(ptr->data, data, sz) == 0)
        {
            return idx;
        }

        ptr = ptr->next;
        idx++;
    }

    return -1;
}

void *list_get_at(list_t* lptr, long atpos)
{
    if(lptr == NULL)
    {
        return NULL;
    }

    if(atpos > (lptr->count - 1) || atpos < 0)
    {
        return NULL;
    }

    node_t* ptr = NULL;
    ptr = lptr->head;

    if(atpos > 0)
    {
        for(size_t idx = 0; idx < atpos; idx++)
        {
            ptr = ptr->next;
        }
    }

    return ptr->data;
}

void* list_get_first(list_t* lptr, size_t* out_size)
{
    if (lptr == NULL || lptr->head == NULL)
    {
        return NULL;
    }

    lptr->iterator = lptr->head;
    if (out_size != NULL)
    {
        *out_size = lptr->iterator->size;
    }
    return lptr->iterator->data;
}

void* list_get_next(list_t* lptr, size_t* out_size)
{
    if (lptr == NULL || lptr->iterator == NULL || lptr->iterator->next == NULL)
    {
        return NULL;
    }

    lptr->iterator = lptr->iterator->next;
    if (out_size != NULL)
    {
        *out_size = lptr->iterator->size;
    }
    return lptr->iterator->data;
}

void* list_get_last(list_t* lptr, size_t* out_size)
{
    if(lptr == NULL || lptr->tail == NULL)
    {
        return NULL;
    }

    lptr->iterator = lptr->tail;
    if (out_size != NULL)
    {
        *out_size = lptr->iterator->size;
    }
    return lptr->iterator->data;
}

list_t* list_sort(list_t* lptr)
{
    if(lptr == NULL || lptr->count < 2)
    {
        return lptr;
    }

    // Sort linked list nodes using merge sort with default comparator
    lptr->head = list_internal_merge_sort_nodes(lptr->head, list_internal_default_node_cmp);

    // Update tail pointer after sorting
    node_t* current = lptr->head;
    while(current->next != NULL)
    {
        current = current->next;
    }
    lptr->tail = current;

    return lptr;
}

list_t* list_merge(list_t* lptrFirst, list_t* lptrSecond)
{
    if(lptrFirst == NULL)
    {
        return lptrSecond;
    }

    if(lptrSecond == NULL)
    {
        return lptrFirst;
    }

    if (lptrFirst == NULL && lptrSecond == NULL)
    {
        return NULL;
    }

    list_t* merged = list_allocate(NULL);
    if (merged == NULL)
    {
        return NULL;
    }

    list_internal_copy_nodes_to_list(merged, lptrFirst);
    list_internal_copy_nodes_to_list(merged, lptrSecond);

    return merged;
}

list_t* list_join(list_t* lptrFirst, list_t* lptrSecond)
{
    if(lptrFirst == NULL)
    {
        return lptrSecond;
    }

    if(lptrSecond == NULL)
    {
        return lptrFirst;
    }

    if (lptrFirst->count == 0)
    {
        // First list empty, just take second's nodes
        free(lptrFirst); // free old list struct
        return lptrSecond;
    }

    if (lptrSecond->count == 0)
    {
        // Second list empty, nothing to do
        return lptrFirst;
    }

    // Join lists by linking tails and heads
    lptrFirst->tail->next = lptrSecond->head;
    lptrFirst->tail = lptrSecond->tail;
    lptrFirst->count += lptrSecond->count;

    // Invalidate second list struct but not nodes
    lptrSecond->head = NULL;
    lptrSecond->tail = NULL;
    lptrSecond->count = 0;

    free(lptrSecond);

    return lptrFirst;
}

void list_internal_copy_nodes_to_list(list_t* dest, list_t* src)
{
    if (src == NULL || dest == NULL)
    {
        return;
    }

    node_t* current = src->head;

    while (current != NULL)
    {
        list_add_to_tail(dest, current->data, current->size);
        current = current->next;
    }
}

void list_internal_remove_from_head(list_t* lptr)
{
    if (lptr == NULL)
    {
        return;
    }

    if (lptr->head == NULL)
    {
        return;
    }

    node_t* oldhead = lptr->head;
    lptr->head = lptr->head->next;

    free(oldhead->data);
    free(oldhead);

    lptr->count--;

    if (lptr->count == 0)
    {
        lptr->tail = NULL;
    }
}

void list_internal_remove_from_tail(list_t* lptr)
{
    if (lptr == NULL)
    {
        return;
    }

    if (lptr->head == NULL)
    {
        return;
    }

    if (lptr->head == lptr->tail)
    {
        free(lptr->head->data);
        free(lptr->head);
        lptr->head = NULL;
        lptr->tail = NULL;
        lptr->count = 0;
        return;
    }

    node_t* cur = lptr->head;
    while (cur->next != lptr->tail)
    {
        cur = cur->next;
    }

    free(lptr->tail->data);
    free(lptr->tail);

    cur->next = NULL;
    lptr->tail = cur;
    lptr->count--;
}

void list_internal_add_to_head(list_t* lptr, node_t* ptr)
{
    ptr->next = NULL; // explicitly clear next pointer

    if(lptr->count == 0)
    {
        lptr->iterator = lptr->head = lptr->tail = ptr;
    }
    else
    {
        ptr->next = lptr->head;
        lptr->head = ptr;
    }

    lptr->count++;
}

void list_internal_add_to_tail(list_t* lptr, node_t* ptr)
{
    ptr->next = NULL; // explicitly clear next pointer

    if(lptr->count == 0)
    {
        lptr->iterator = lptr->head = lptr->tail = ptr;
    }
    else
    {
        lptr->tail->next = ptr;
        lptr->tail = ptr;
    }

    lptr->count++;
}

void list_internal_split(node_t* source, node_t** frontRef, node_t** backRef)
{
    node_t* slow;
    node_t* fast;

    slow = source;
    fast = source->next;

    while(fast != NULL)
    {
        fast = fast->next;
        if(fast != NULL)
        {
            slow = slow->next;
            fast = fast->next;
        }
    }

    *frontRef = source;
    *backRef = slow->next;
    slow->next = NULL;
}

node_t* list_internal_sorted_merge(node_t* a, node_t* b, list_cmp_fn cmp)
{
    node_t* result = NULL;

    if(a == NULL)
        return b;
    else if(b == NULL)
        return a;

    int cmpres = cmp(a->data, a->size, b->data, b->size);

    if(cmpres <= 0)
    {
        result = a;
        result->next = list_internal_sorted_merge(a->next, b, cmp);
    }
    else
    {
        result = b;
        result->next = list_internal_sorted_merge(a, b->next, cmp);
    }

    return result;
}

node_t* list_internal_merge_sort_nodes(node_t* head, list_cmp_fn cmp)
{
    if(head == NULL || head->next == NULL)
    {
        return head;
    }

    node_t* a;
    node_t* b;

    list_internal_split(head, &a, &b);

    a = list_internal_merge_sort_nodes(a, cmp);
    b = list_internal_merge_sort_nodes(b, cmp);

    return list_internal_sorted_merge(a, b, cmp);
}

int list_internal_default_node_cmp(const void* dataA, size_t sizeA, const void* dataB, size_t sizeB)
{
    size_t min_size = sizeA < sizeB ? sizeA : sizeB;
    int cmp = memcmp(dataA, dataB, min_size);

    if (cmp == 0)
    {
        if (sizeA < sizeB)
            return -1;
        else if (sizeA > sizeB)
            return 1;
        else
            return 0;
    }
    return cmp;
}

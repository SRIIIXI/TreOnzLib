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

#include "listdoublelinked.h"
#include "buffer.h"

#include <string.h>
#include <memory.h>
#include <stdlib.h>
#include <limits.h>

typedef int (*list_double_linked_compare_fn)(const void* a, size_t a_sz, const void* b, size_t b_sz);

typedef struct node_double_linked_t
{
    void* data;
    size_t size;
    struct node_double_linked_t* next;
    struct node_double_linked_t* previous;
}node_double_linked_t;

typedef struct list_double_linked_t
{
    long count;
    node_double_linked_t* head;
    node_double_linked_t* tail;
    node_double_linked_t* iterator;
}list_double_linked_t;

void list_double_linked_internal_remove_from_head(list_double_linked_t* lptr);
void list_double_linked_internal_remove_from_tail(list_double_linked_t* lptr);
void list_double_linked_internal_add_to_head(list_double_linked_t* lptr, node_double_linked_t *ptr);
void list_double_linked_internal_add_to_tail(list_double_linked_t* lptr, node_double_linked_t* ptr);
void list_double_linked_internal_append_all(list_double_linked_t* dest, list_double_linked_t* src);

// Forward declarations
static node_double_linked_t* list_double_linked_internal_merge_sorted(node_double_linked_t* left, node_double_linked_t* right, list_double_linked_compare_fn cmp);
static void list_double_linked_internal_split(node_double_linked_t* source, node_double_linked_t** frontRef, node_double_linked_t** backRef);
static list_double_linked_t* list_double_linked_internal_sort(list_double_linked_t* lptr, list_double_linked_compare_fn cmp);
static node_double_linked_t* list_double_linked_internal_merge_sort_recursive(node_double_linked_t* head, list_double_linked_compare_fn cmp);
static void list_double_linked_internal_split(node_double_linked_t* source,node_double_linked_t** frontRef,node_double_linked_t** backRef);
static node_double_linked_t* list_double_linked_internal_merge_sorted( node_double_linked_t* left, node_double_linked_t* right, list_double_linked_compare_fn cmp);
static int list_double_linked_internal_default_compare(const void* a, size_t a_sz, const void* b, size_t b_sz);

list_double_linked_t * list_double_linked_allocate(list_double_linked_t* lptr)
{
    lptr = (list_double_linked_t*)calloc(1, sizeof(list_double_linked_t));

    if (!lptr)
    {
        return NULL;
    }

    lptr->count = 0;
    lptr->head = lptr->tail = NULL;
    lptr->iterator = NULL;
    return lptr;
}

void list_double_linked_clear(list_double_linked_t* lptr)
{
    if(lptr == NULL)
    {
        return;
    }
    else
    {
        while(lptr->count > 0)
        {
            list_double_linked_internal_remove_from_tail(lptr);
        }
    }
}

void list_double_linked_free(list_double_linked_t* lptr)
{
    if(lptr == NULL)
    {
        return;
    }
    else
    {
        while(lptr->count > 0)
        {
            list_double_linked_internal_remove_from_tail(lptr);
        }
        free(lptr);
    }
}

void list_double_linked_add_to_head(list_double_linked_t* lptr, void* data, size_t sz)
{
    list_double_linked_insert(lptr, data, sz, 0);
}

void list_double_linked_add_to_tail(list_double_linked_t* lptr, void* data, size_t sz)
{
    list_double_linked_insert(lptr, data, sz, LONG_MAX);
}

void list_double_linked_insert(list_double_linked_t* lptr, void* data, size_t sz, long pos)
{
    if (lptr == NULL || data == NULL || sz == 0)
    {
        return;
    }
    
    node_double_linked_t* ptr = calloc(1, sizeof(node_double_linked_t));
    
    if (!ptr)
    {
        return;
    }
    
    ptr->data = calloc(1, sz);
    
    if (!ptr->data)
    {
        free(ptr);
        return;
    }
    
    memcpy(ptr->data, data, sz);
    ptr->size = sz;
    
    if (pos <= 0)
    {
        list_double_linked_internal_add_to_head(lptr, ptr);
        return;
    }
    
    if (pos >= lptr->count)
    {
        list_double_linked_internal_add_to_tail(lptr, ptr);
        return;
    }
    
    node_double_linked_t* current = lptr->head;
    
    for (long i = 0; i < pos - 1; i++)
    {
        current = current->next;
    }
    
    ptr->next = current->next;
    ptr->previous = current;
    
    if (current->next != NULL)
    {
        current->next->previous = ptr;
    }
    
    current->next = ptr;
    
    lptr->count++;
}

void list_double_linked_remove_from_head(list_double_linked_t* lptr)
{
    list_double_linked_remove_at(lptr, 0);
}

void list_double_linked_remove_from_tail(list_double_linked_t* lptr)
{
    list_double_linked_remove_at(lptr, LONG_MAX);
}

void list_double_linked_remove(list_double_linked_t* lptr, const void *data)
{
    if (lptr == NULL || data == NULL)
    {
        return;
    }

    node_double_linked_t* curptr = lptr->head;

    while (curptr != NULL)
    {
        if (memcmp(data, curptr->data, curptr->size) == 0)
        {
            if (curptr == lptr->head)
            {
                list_double_linked_internal_remove_from_head(lptr);
                return;
            }

            if (curptr == lptr->tail)
            {
                list_double_linked_internal_remove_from_tail(lptr);
                return;
            }

            // Middle node unlink
            curptr->previous->next = curptr->next;
            curptr->next->previous = curptr->previous;

            free(curptr->data);
            free(curptr);

            lptr->count--;
            return;
        }

        curptr = curptr->next;
    }
}

void list_double_linked_remove_at(list_double_linked_t* lptr, long pos)
{
    if (lptr == NULL || pos < 0 || pos >= lptr->count)
    {
        return;
    }

    if (pos == 0)
    {
        list_double_linked_internal_remove_from_head(lptr);
        return;
    }

    if (pos == lptr->count - 1)
    {
        list_double_linked_internal_remove_from_tail(lptr);
        return;
    }

    node_double_linked_t* curptr = lptr->head;

    for (long idx = 0; idx < pos; idx++)
    {
        curptr = curptr->next;
    }

    // Unlink node
    curptr->previous->next = curptr->next;
    curptr->next->previous = curptr->previous;

    free(curptr->data);
    free(curptr);

    lptr->count--;
}

void list_double_linked_remove_value(list_double_linked_t* lptr, void* data, size_t sz)
{
    if (lptr == NULL || data == NULL || sz == 0)
    {
        return;
    }

    node_double_linked_t* ptr = lptr->head;
    long idx = 0;

    while (ptr != NULL)
    {
        if (ptr->size == sz && memcmp(ptr->data, data, sz) == 0)
        {
            if (ptr == lptr->head)
            {
                list_double_linked_internal_remove_from_head(lptr);
                return;
            }

            if (ptr == lptr->tail)
            {
                list_double_linked_internal_remove_from_tail(lptr);
                return;
            }

            ptr->previous->next = ptr->next;
            ptr->next->previous = ptr->previous;

            free(ptr->data);
            free(ptr);

            lptr->count--;
            return;
        }

        ptr = ptr->next;
        idx++;
    }
}

long list_double_linked_item_count(list_double_linked_t* lptr)
{
    if(lptr != NULL)
    {
        return lptr->count;
    }

    return 0;
}

long list_double_linked_index_of(list_double_linked_t *lptr, const void *node)
{
    if(lptr == NULL)
    {
        return -1;
    }

    node_double_linked_t* ptr = NULL;

    ptr = lptr->head;

    long idx = 0;

    if(ptr->data == node)
    {
        return idx;
    }

    while(true)
    {
        if(ptr == NULL)
        {
            break;
        }

        ptr = ptr->next;
        idx++;

        if(ptr->data == node)
        {
            return idx;
        }
    }

    return -1;
}

long list_double_linked_index_of_value(list_double_linked_t* lptr, void* data, size_t sz)
{
    if(lptr == NULL)
    {
        return -1;
    }

    node_double_linked_t* ptr = NULL;

    ptr = lptr->head;

    long idx = 0;

    if(memcmp(ptr->data, data, ptr->size) == 0 && ptr->size == sz)
    {
        return idx;
    }

    while(true)
    {
        if(ptr == NULL)
        {
            break;
        }

        ptr = ptr->next;
        idx++;

        if(memcmp(ptr->data, data, ptr->size) == 0)
        {
            return idx;
        }
    }

    return -1;
}

void *list_double_linked_get_at(list_double_linked_t* lptr, long atpos)
{
    if(lptr == NULL)
    {
        return NULL;
    }

    if(atpos > (lptr->count - 1) || atpos < 0)
    {
        return NULL;
    }

    node_double_linked_t* ptr = NULL;

    ptr = lptr->head;

    if(atpos > 0)
    {
        for(int idx = 0; idx < atpos; idx++)
        {
            ptr = ptr->next;
        }
    }

    return ptr->data;
}

void* list_double_linked_get_first(list_double_linked_t* lptr)
{
    if(lptr == NULL)
    {
        return NULL;
    }

    lptr->iterator = lptr->head;
    return lptr->iterator->data;
}

void* list_double_linked_get_next(list_double_linked_t* lptr)
{
    if (lptr == NULL || lptr->iterator == NULL)
    {
        return NULL;
    }

    if (lptr->iterator->next == NULL)
    {
        return NULL;
    }

    lptr->iterator = lptr->iterator->next;

    return lptr->iterator->data;
}

void* list_double_linked_get_last(list_double_linked_t* lptr)
{
    if(lptr == NULL|| lptr->iterator == NULL)
    {
        return NULL;
    }

    lptr->iterator = lptr->tail;
    return lptr->iterator->data;
}

void* list_double_linked_get_previous(list_double_linked_t* lptr)
{
    if(lptr == NULL)
    {
        return NULL;
    }

    if(lptr->iterator->previous == NULL)
    {
        return NULL;
    }

    lptr->iterator = lptr->iterator->previous;

    return lptr->iterator->data;
}

list_double_linked_t* list_double_linked_sort(list_double_linked_t* lptr)
{
    if(lptr == NULL)
    {
        return NULL;
    }

    if (lptr == NULL)
    {
        return NULL;
    }

    return list_double_linked_internal_sort(lptr, list_double_linked_internal_default_compare);
}

list_double_linked_t* list_double_linked_merge(list_double_linked_t* lptrFirst, list_double_linked_t* lptrSecond)
{
    if (lptrFirst == NULL && lptrSecond == NULL)
    {
        return NULL;
    }

    list_double_linked_t* lptrNew = list_double_linked_join(lptrFirst, lptrSecond);

    if (lptrNew == NULL)
    {
        return NULL;
    }

    // Call the sort stub here
    return list_double_linked_sort(lptrNew);
}

list_double_linked_t* list_double_linked_join(list_double_linked_t* lptrFirst, list_double_linked_t* lptrSecond)
{
    if (lptrFirst == NULL && lptrSecond == NULL)
    {
        return NULL;
    }

    list_double_linked_t* lptrNew = list_double_linked_allocate(NULL);
    
    if (lptrNew == NULL)
    {
        return NULL;
    }

    if (lptrFirst != NULL)
    {
        list_double_linked_internal_append_all(lptrNew, lptrFirst);
    }

    if (lptrSecond != NULL)
    {
        list_double_linked_internal_append_all(lptrNew, lptrSecond);
    }

    return lptrNew;
}

void list_double_linked_internal_remove_from_head(list_double_linked_t* lptr)
{
    if (lptr->head == NULL)
    {
        return;
    }
    
    node_double_linked_t* oldhead = lptr->head;
    lptr->head = oldhead->next;
    
    if (lptr->head != NULL)
    {
        lptr->head->previous = NULL;
    }
    else
    {
        lptr->tail = NULL;  // List became empty
    }
    
    free(oldhead->data);
    free(oldhead);
    
    lptr->count--;
}

void list_double_linked_internal_remove_from_tail(list_double_linked_t* lptr)
{
    if (lptr->tail == NULL)
    {
        return;
    }
    
    node_double_linked_t* oldtail = lptr->tail;
    lptr->tail = oldtail->previous;
    
    if (lptr->tail != NULL)
    {
        lptr->tail->next = NULL;
    }
    else
    {
        lptr->head = NULL;  // List became empty
    }
    
    free(oldtail->data);
    free(oldtail);
    
    lptr->count--;
}

void list_double_linked_internal_add_to_head(list_double_linked_t* lptr, node_double_linked_t* ptr)
{
    ptr->previous = NULL;
    ptr->next = lptr->head;
    
    if (lptr->head != NULL)
    {
        lptr->head->previous = ptr;
    }
    
    lptr->head = ptr;
    
    if (lptr->tail == NULL)
    {
        lptr->tail = ptr;
    }
    
    lptr->count++;
}

void list_double_linked_internal_add_to_tail(list_double_linked_t* lptr, node_double_linked_t* ptr)
{
    ptr->next = NULL;
    ptr->previous = lptr->tail;
    
    if (lptr->tail != NULL)
    {
        lptr->tail->next = ptr;
    }
    
    lptr->tail = ptr;
    
    if (lptr->head == NULL)
    {
        lptr->head = ptr;
    }
    
    lptr->count++;
}

list_double_linked_t* list_double_linked_internal_sort(list_double_linked_t* lptr, list_double_linked_compare_fn cmp)
{
    if (lptr == NULL || lptr->count <= 1 || cmp == NULL)
    {
        return lptr;
    }

    // Recursively sort starting from head
    lptr->head = list_double_linked_internal_merge_sort_recursive(lptr->head, cmp);

    // Reset tail and fix previous pointers
    node_double_linked_t* current = lptr->head;
    lptr->tail = NULL;

    while (current != NULL)
    {
        if (current->next == NULL)
        {
            lptr->tail = current;
        }
        else
        {
            current->next->previous = current;
        }
        current = current->next;
    }

    lptr->iterator = NULL;

    return lptr;
}

node_double_linked_t* list_double_linked_internal_merge_sort_recursive(
    node_double_linked_t* head,
    list_double_linked_compare_fn cmp)
{
    if (head == NULL || head->next == NULL)
    {
        return head;
    }

    node_double_linked_t* left = NULL;
    node_double_linked_t* right = NULL;

    list_double_linked_internal_split(head, &left, &right);

    left = list_double_linked_internal_merge_sort_recursive(left, cmp);
    right = list_double_linked_internal_merge_sort_recursive(right, cmp);

    return list_double_linked_internal_merge_sorted(left, right, cmp);
}

void list_double_linked_internal_split(
    node_double_linked_t* source,
    node_double_linked_t** frontRef,
    node_double_linked_t** backRef)
{
    node_double_linked_t* slow = source;
    node_double_linked_t* fast = source->next;

    while (fast != NULL)
    {
        fast = fast->next;
        if (fast != NULL)
        {
            slow = slow->next;
            fast = fast->next;
        }
    }

    *frontRef = source;
    *backRef = slow->next;
    slow->next = NULL;

    if (*backRef)
    {
        (*backRef)->previous = NULL;
    }
}

node_double_linked_t* list_double_linked_internal_merge_sorted(
    node_double_linked_t* left,
    node_double_linked_t* right,
    list_double_linked_compare_fn cmp)
{
    if (left == NULL)
    {
        return right;
    }

    if (right == NULL)
    {
        return left;
    }

    node_double_linked_t* result = NULL;

    if (cmp(left->data, left->size, right->data, right->size) <= 0)
    {
        result = left;
        result->next = list_double_linked_internal_merge_sorted(left->next, right, cmp);
        if (result->next != NULL)
        {
            result->next->previous = result;
        }
        result->previous = NULL;
    }
    else
    {
        result = right;
        result->next = list_double_linked_internal_merge_sorted(left, right->next, cmp);
        if (result->next != NULL)
        {
            result->next->previous = result;
        }
        result->previous = NULL;
    }

    return result;
}

int list_double_linked_internal_default_compare(const void* a, size_t a_sz, const void* b, size_t b_sz)
{
    if (a_sz != sizeof(int) || b_sz != sizeof(int)) 
    {
        // Fallback or error handling
        return 0;
    }
    int int_a = *(const int*)a;
    int int_b = *(const int*)b;

    if (int_a < int_b) return -1;
    if (int_a > int_b) return 1;
    return 0;
}

// Helper to append all nodes from a list to lptrNew
void list_double_linked_internal_append_all(list_double_linked_t* dest, list_double_linked_t* src)
{
    node_double_linked_t* ptr = src->head;
    while (ptr != NULL)
    {
        list_double_linked_add_to_tail(dest, ptr->data, ptr->size);
        ptr = ptr->next;
    }
}

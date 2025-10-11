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

#include "stringex.h"
#include <memory.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <stdarg.h>

typedef struct string_t
{
    char* data;
    size_t data_size;
    size_t memory_size;
}string_t;

typedef struct string_list_t
{
    string_t* strings;
    size_t num_of_strings;
    size_t current_index;
}string_list_t;

string_t* string_internal_adjust_storage(string_t* string_ptr, size_t sz);
char* string_internal_from_int(long num);
char* string_internal_from_double(double num);

string_t* string_allocate(const char* data)
{
    if(data == NULL)
    {
        return NULL;
    }

    string_t* nd = (string_t*)calloc(1, sizeof(string_t));

    if(nd != NULL)
    {
        size_t slen = strlen(data);
        size_t psize = sysconf(_SC_PAGESIZE);

        size_t pcount = slen/psize;

        if(slen%psize != 0)
        {
            pcount++;
        }

        nd->memory_size = psize*pcount;
        nd->data_size = slen;
        nd->data = (char*)calloc(nd->memory_size, sizeof (char));

        if(nd->data != NULL)
        {
            memcpy(nd->data, data, slen);
        }
    }
    return nd;
}

string_t* string_allocate_default(void)
{
    string_t* nd = (string_t*)calloc(1, sizeof(string_t));

    if(nd != NULL)
    {
        nd->memory_size = sysconf(_SC_PAGESIZE);
        nd->data_size = 0;
        nd->data = (char*)calloc(nd->memory_size, sizeof (char));
    }
    return nd;
}

string_t* string_allocate_length(size_t slen)
{
    string_t* nd = (string_t*)calloc(1, sizeof(string_t));
    if(nd != NULL)
    {
        size_t psize = sysconf(_SC_PAGESIZE);

        size_t pcount = slen/psize;

        if(slen%psize != 0)
        {
            pcount++;
        }

        nd->memory_size = psize*pcount;

        nd->data_size = 0;
        nd->data = (char*)calloc(nd->memory_size, sizeof (char));
    }
    return nd;
}

string_t* string_allocate_formatted(const char* format, ...)
{
    if(format == NULL)
    {
        return NULL;
    }
    
    va_list args;
    va_list args_copy;
    
    // First pass: determine required buffer size
    va_start(args, format);
    va_copy(args_copy, args);
    
    int needed_size = vsnprintf(NULL, 0, format, args);
    va_end(args);
    
    if(needed_size < 0)
    {
        va_end(args_copy);
        return NULL;
    }
    
    // Allocate string_t with appropriate size
    string_t* result = string_allocate_length(needed_size + 1);
    
    if(result == NULL)
    {
        va_end(args_copy);
        return NULL;
    }
    
    // Second pass: actually format into the buffer
    vsnprintf(result->data, needed_size + 1, format, args_copy);
    va_end(args_copy);
    
    // Update data_size
    result->data_size = needed_size;
    
    return result;
}

void string_free(string_t** str)
{
    if(str == NULL || *str == NULL)
    {
        return;
    }

    free((*str)->data);
    (*str)->data = NULL;
    free(*str);
    *str = NULL; 
}

void string_clear(string_t* str)
{
    if(str == NULL)
    {
        return;
    }

    if(str->data)
    {
        for(size_t i = 0; i < str->data_size; ++i)
            str->data[i] = 0;
    }

    str->data_size = 0;
}

bool string_is_equal(const string_t* first, const string_t* second)
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

bool string_is_greater(const string_t* first, const string_t* second)
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

bool string_is_less(const string_t* first, const string_t* second)
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

bool string_is_null(const string_t* ptr)
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

size_t string_get_length(const string_t* str)
{
    return str->data_size;
}

const char* string_c_str(const string_t* str)
{
    return str->data;
}

wchar_t *string_c_to_wstr(const char *str)
{
    if (!str)
    {
        return NULL;
    }

    size_t len = mbstowcs(NULL, str, 0);
    if (len == (size_t)-1)
    {
        return NULL; // conversion failed
    }

    wchar_t *wstr = malloc((len + 1) * sizeof(wchar_t));
    if (!wstr)
    {
        return NULL;
    }

    mbstowcs(wstr, str, len + 1);
    return wstr;
}

wchar_t* string_to_wstr(const string_t* str)
{
    return string_c_to_wstr(str->data);
}

string_t *string_from_wstr(const wchar_t *wstr)
{
	if(wstr == NULL)
	{
		return NULL;
	}

    long wlen = 0;
    char* str = NULL;

    for (wlen = 0; wstr[wlen] != '\0'; wlen++) {}

    str = (char*)calloc(1, (unsigned long)wlen+1);

    if (str != NULL)
    {
        for (long idx = 0; idx < wlen; idx++)
        {
            str[idx] = (char)wstr[idx];
        }
    }

    string_t* ptr = string_allocate(str);

    free(str);

    return ptr;
}


string_t* string_copy(string_t* dest, string_t* orig)
{
    if(orig->data_size > dest->data_size)
    {
        string_internal_adjust_storage(dest, orig->data_size);
    }

    // Clear destination buffer
    for(size_t ctr = 0; ctr < dest->memory_size; ctr++)
    {
        dest->data[ctr] = 0;
    }
    
    // Deep copy the data
    for(size_t ctr = 0; ctr < dest->memory_size; ctr++)
    {
        dest->data[ctr] = orig->data[ctr];
    }    
    // Update destination size to match source
    dest->data_size = orig->data_size;
    
    return dest; 
}

string_t* string_append(string_t* dest, const char *data)
{
    if(data == NULL)
    {
        return NULL;
    }

    if(strlen(data) < 1)
    {
        return NULL;
    }

    if(dest == NULL)
    {
        dest = string_allocate(data);
        return dest;
    }
    else
    {
        size_t sz = strlen(data);
        dest = string_internal_adjust_storage(dest, sz);
        memcpy(&dest->data[dest->data_size], data, sz);
        dest->data_size = dest->data_size + sz;
    }

    return dest;
}

string_t* string_append_string(string_t* dest, const string_t *str)
{
    return string_append(dest, str->data);
}

string_t* string_append_integer(string_t* dest, const long data)
{
    char buffer[33] = {0};
    sprintf(buffer, "%ld", data);
    return string_append(dest, buffer);
}

string_t* string_append_real(string_t* dest, const double data)
{
    char buffer[33] = {0};
    sprintf(buffer, "%f", data);
    return string_append(dest, buffer);
}

string_t* string_append_real_scientific(string_t* dest, const double data)
{
    char buffer[33] = {0};
    sprintf(buffer, "%.3e", data);
    return string_append(dest, buffer);
}

string_t* string_append_char(string_t* dest, const char data)
{
    char buffer[2] = {data, 0};
    return string_append(dest, buffer);
}

string_t* string_append_boolean(string_t* dest, const bool data)
{
    char buffer[6] = {0};

    if(data)
    {
        strcpy(buffer, "true");
    }
    else
    {
        strcpy(buffer, "false");
    }

    return string_append(dest, buffer);
}

string_t* string_append_curr_timestamp(string_t* dest)
{
    if(dest == NULL)
    {
        return NULL;
    }

    char buffer[129] = {0};
    time_t t ;
    struct tm *tmp ;
    time(&t);
    tmp = localtime(&t);

    sprintf(buffer, "%04d%02d%02d%02d%02d%02d",
            (tmp->tm_year+1900), (tmp->tm_mon+1), tmp->tm_mday,
            tmp->tm_hour, tmp->tm_min, tmp->tm_sec);

    return string_append(dest, buffer);
}

string_t *string_reverse(string_t *str)
{
	size_t start = 0;

    size_t term = str->data_size - 1;

	while (start < term)
	{
        char temp = str->data[start];
        str->data[start] = str->data[term];
        str->data[term] = temp;
		start++;
		term--;
	}

    return str;
}

string_t *string_segment_reverse(string_t *str, size_t start, size_t term)
{
	while (start < term)
	{
        char temp = str->data[start];
        str->data[start] = str->data[term];
        str->data[term] = temp;
		start++;
		term--;
	}

    return str;
}

long string_index_of_substr(const string_t *str, const string_t *substr)
{
    if(str == NULL || substr == NULL)
    {
        return -1;
    }

    if(str->data == NULL || substr->data == NULL)
    {
        return -1;
    }

    long result = -1;

    char* pdest = (char*)strstr( str->data, substr->data);

    if(pdest == 0)
    {
        return -1;
    }

    result = (long)(pdest - str->data);

    return result;
}

long string_index_of_char(const string_t *str, const char ch)
{
    for (int ctr = 0; str->data[ctr] != '\0'; ctr++)
    {
        if (str->data[ctr] == ch)
        {
            return ctr;
        }
    }

    return -1;
}

long string_count_substr(const string_t *str, const string_t *substr)
{
    if(str == NULL || substr == NULL)
    {
        return 0;
    }

    const char* string = string_c_str(str);
    const char* substring = string_c_str(substr);
    size_t count = 0;

    if (!string || !substring) 
    {
        return 0;
    }
    
    size_t sub_len = strlen(substring);
    if (sub_len == 0) 
    {
        return 0;
    }
    
    const char* pos = string;
    
    while ((pos = strstr(pos, substring)) != NULL) 
    {
        count++;
        pos += sub_len;  // Move past this occurrence
    }
    
    return count;
}

long string_count_char(const string_t *str, const char ch)
{
    long ctr = 0;

    for (long index = 0; str->data[index] != '\0'; index++)
	{
        if (str->data[index] == ch)
		{
			ctr++;
		}
	}

	return ctr;
}

string_t *string_to_lower(string_t *str)
{
    for (long ctr = 0; str->data[ctr] != '\0'; ctr++)
    {
        if (str->data[ctr] >= 65 && str->data[ctr] <= 90)
        {
            str->data[ctr] = str->data[ctr] + 32;
        }
    }

    return str;
}

extern string_t *string_to_upper(string_t *str)
{
    for (long ctr = 0; str->data[ctr] != '\0'; ctr++)
    {
        if (str->data[ctr] >= 97 && str->data[ctr] <= 122)
        {
            str->data[ctr] = str->data[ctr] - 32;
        }
    }

    return str;
}

string_t *string_left_trim(string_t *str)
{
    char *ptr = str->data;

    long ctr = 0;

    while (isspace(*ptr))
    {
        ptr++;
    }

    while (*ptr)
    {
        str->data[ctr] = *ptr;
        ctr++;
        ptr++;
    }

    while (str->data[ctr] != '\0')
	{
        str->data[ctr] = '\0';
		ctr++;
	}

    str->data_size = strlen(str->data);

    return str;
}

string_t *string_right_trim(string_t *str)
{
    long len = str->data_size;

    for (long ctr = len - 1; ctr > -1; ctr--)
    {
        if (isspace(str->data[ctr]))
        {
            str->data[ctr] = '\0';
        }
        else
        {
            break;
        }
    }

    str->data_size = strlen(str->data);

    return str;
}

string_t *string_all_trim(string_t *str)
{
    string_right_trim(str);
    string_left_trim(str);
    return str;
}

string_t *string_remove_substr_first(string_t *str, const string_t *substr)
{
    long pos = -1;
    long offset = substr->data_size;

    pos = string_index_of_substr(str, substr);

    if(pos >= 0)
    {
        strcpy(str->data + pos, str->data + pos + offset);
        str->data[str->data_size - (unsigned long)offset] = 0;
    }

    str->data_size = strlen(str->data);

    return str;
}

string_t *string_remove_substr_all(string_t *str, const string_t *substr)
{
    long pos = -1;
    long offset = substr->data_size;

    pos = string_index_of_substr(str, substr);

    while(pos >= 0)
    {
        strcpy(str->data + pos, str->data + pos + offset);
        str->data[str->data_size - (unsigned long)offset] = 0;
        pos = string_index_of_substr(str, substr);
    }

    str->data_size = strlen(str->data);

    return str;
}

string_t* string_remove_substr_at(string_t *str, size_t pos, size_t len)
{
    if(pos >= 0 && pos <= (str->data_size-1) )
    {
        strcpy(str->data + pos, str->data + pos + len);
        str->data[str->data_size - len] = 0;
    }

    str->data_size = strlen(str->data);

    return str;
}

void string_remove_end(string_t* str, size_t len)
{
    if (str == NULL || str->data == NULL)
    {
        return;
    }

    size_t cur_len = strlen(str->data);

    if (len >= cur_len)
    {
        // Remove everything
        memset(str->data, '\0', str->data_size);
        return;
    }

    // Null terminate after removing 'len' chars
    size_t new_len = cur_len - len;
    str->data[new_len] = '\0';

    // Zero out the remainder of the buffer
    if (str->data_size > new_len + 1)
    {
        memset(str->data + new_len + 1, '\0',
               str->data_size - (new_len + 1));
    }

    str->data_size = strlen(str->data);
}

void string_remove_start(string_t* str, size_t len)
{
    if (str == NULL || str->data == NULL)
    {
        return;
    }

    size_t cur_len = strlen(str->data);

    if (len == 0 || cur_len == 0)
    {
        return;
    }

    if (len >= cur_len)
    {
        // Remove everything
        memset(str->data, '\0', str->data_size);
        return;
    }

    // Shift remaining characters to the front
    memmove(str->data, str->data + len, cur_len - len + 1);

    // Zero out the remainder of the buffer
    size_t new_len = strlen(str->data);
    if (str->data_size > new_len + 1)
    {
        memset(str->data + new_len + 1, '\0',
               str->data_size - (new_len + 1));
    }

    str->data_size = strlen(str->data);
}

string_t *string_remove_char_first(string_t *str, const char oldchar)
{
    if (str == NULL || str->data == NULL)
    {
        return str;
    }

    size_t i = 0;
    size_t cur_len = 0;
    int found = 0;

    // Loop over each character until null terminator
    while (str->data[i] != '\0')
    {
        if (!found && str->data[i] == oldchar)
        {
            found = 1; // Mark first match
        }

        if (found)
        {
            // Shift next character into current position
            str->data[i] = str->data[i + 1];
        }

        i++;
    }

    cur_len = i;

    // Clear remaining unused space in buffer
    for (size_t j = cur_len + 1; j < str->data_size; j++)
    {
        str->data[j] = '\0';
    }

    str->data_size = strlen(str->data);

    return str;
}

string_t *string_remove_char_all(string_t *str, const char oldchar)
{
    long pos = string_index_of_char(str, oldchar);

    while(pos >= 0)
    {
        strcpy(str->data + pos, str->data+pos + 1);
        str->data[str->data_size - 1] = 0;
        pos = string_index_of_char(str, oldchar);
    }

    str->data_size = strlen(str->data);

    return str;
}

string_t *string_remove_char_at(string_t *str, size_t pos)
{
    strcpy(str->data + pos, str->data + pos + 1);
    str->data[str->data_size - 1] = 0;
    str->data_size = strlen(str->data);
    return str;
}

string_t *string_replace_substr_first(string_t *str, const string_t *oldsubstr, const string_t *newsubstr)
{
    if(str == NULL || oldsubstr == NULL || newsubstr == NULL)
    {
        return NULL;
    }

    long pos = string_index_of_substr(str, oldsubstr);
    if(pos < 0)
    {
        return str;  // No match found
    }

    long slen = str->data_size;
    long oldslen = oldsubstr->data_size;
    long newslen = newsubstr->data_size;

    if(oldslen < 1)
    {
        return NULL;
    }

    long new_size = slen - oldslen + newslen;
    
    // Adjust storage if needed (expand)
    if(new_size > str->memory_size)
    {
        string_internal_adjust_storage(str, new_size - str->data_size);
    }

    // Shift data if replacement size differs
    if(newslen != oldslen)
    {
        long shift = newslen - oldslen;
        if(shift > 0)
        {
            // Shift right - start from end to avoid overwrite
            for(long i = slen; i >= pos + oldslen; i--)
            {
                str->data[i + shift] = str->data[i];
            }
        }
        else
        {
            // Shift left
            for(long i = pos + oldslen; i <= slen; i++)
            {
                str->data[i + shift] = str->data[i];
            }
        }
    }

    // Copy new substring
    for(long i = 0; i < newslen; i++)
    {
        str->data[pos + i] = newsubstr->data[i];
    }
    
    str->data_size = new_size;
    
    // Shrink if wasting significant memory
    string_internal_adjust_storage(str, 0);
    
    return str;
}

string_t *string_replace_substr_all(string_t *str, const string_t *oldsubstr, const string_t *newsubstr)
{
    if(str == NULL || oldsubstr == NULL || newsubstr == NULL)
    {
        return NULL;
    }

    if(oldsubstr->data_size < 1)
    {
        return str; // Cannot replace an empty string
    }

    // Optimization: If old and new substrings are the same size, we can avoid repeated index lookups by shifting the search window.
    // If they are different sizes, we must re-scan from the beginning after each replacement, as the base string shifts.
    long pos = -1;
    long search_start_index = 0;
    
    // We will use string_index_of_substr repeatedly.
    // To implement the replacement logic cleanly and handle memory, we'll
    // manually implement the replacement logic from string_replace_substr_first,
    // or if we could rely on string_replace_substr_first not returning NULL on error, we could call it.
    // Let's implement the logic here to control the search start point.

    long slen = str->data_size;
    long oldslen = oldsubstr->data_size;
    long newslen = newsubstr->data_size;

    // Use a while loop to find and replace all occurrences
    while ( (pos = string_index_of_substr(str, oldsubstr)) >= 0 ) 
    {
        // Re-calculate lengths as 'str' may have been resized in previous loop iterations
        slen = str->data_size; 
        
        long new_size = slen - oldslen + newslen;
        
        // Adjust storage if needed (expand). The function must return the potentially reallocated pointer.
        str = string_internal_adjust_storage(str, new_size - str->data_size);
        if (str == NULL) 
        {
             return NULL; // Allocation failure
        }
        
        // Re-calculate lengths and position in case of reallocation
        slen = str->data_size; 
        
        // Shift data if replacement size differs
        if(newslen != oldslen)
        {
            long shift = newslen - oldslen;
            if(shift > 0)
            {
                // Shift right - start from end to avoid overwrite
                // We need to shift everything *after* the match.
                // The shift starts from the old end (slen) and ends at the oldslen start (pos + oldslen)
                for(long i = slen; i >= pos + oldslen; i--)
                {
                    str->data[i + shift] = str->data[i];
                }
            }
            else // shift <= 0 (shift left)
            {
                // Shift left
                // We need to shift everything *after* the match.
                // The shift starts from the end of the old match (pos + oldslen) and ends at the old end (slen)
                for(long i = pos + oldslen; i <= slen; i++)
                {
                    str->data[i + shift] = str->data[i];
                }
            }
        }
    
        // Copy new substring
        for(long i = 0; i < newslen; i++)
        {
            str->data[pos + i] = newsubstr->data[i];
        }
        
        // Update data_size
        str->data_size = new_size;
        
        // Null-terminate the new string (Crucial for string_index_of_substr and C-string functions)
        str->data[str->data_size] = '\0';
        
        // Optional: Shrink if wasting significant memory (already in string_internal_adjust_storage, but called with sz=0)
        // string_internal_adjust_storage(str, 0); 

        // Set the next search start point to the end of the newly inserted substring
        // This is important to avoid infinite loops when oldsubstr is a substring of newsubstr (e.g., replacing "a" with "aa")
        search_start_index = pos + newslen;
    }

    return str;
}

string_t *string_replace_char_first(string_t *str, const char oldchar, const char newchar)
{
	if(str != NULL)
	{
        for(size_t pos = 0; str->data[pos] != 0; pos++)
		{
            if(str->data[pos] == oldchar)
			{
                str->data[pos] = newchar;
				return str;
			}
		}
		return str;
	}
	return NULL;
}

string_t *string_replace_char_all(string_t *str, const char oldchar, const char newchar)
{
    if(str != NULL)
    {
        for(size_t pos = 0; str->data[pos] != 0; pos++)
        {
            if(str->data[pos] == oldchar)
            {
                str->data[pos] = newchar;
            }
        }
        return str;
    }
    return NULL;
}

string_t *string_replace_char_at(string_t *str, const char newchar, size_t pos)
{
    if(str != NULL)
    {
        if(pos < str->data_size)
        {
            str->data[pos] = newchar;
            return str;
        }
    }

    return NULL;
}

void string_split_key_value_by_char(const string_t *str, const char delimiter, string_t **key, string_t **value)
{
    if(str == NULL || delimiter == 0)
    {
        return;
    }

    long pos = string_index_of_char(str, delimiter);

    if(pos < 0)
    {
        return;
    }

    long val_start = pos + 1;
    long val_end = str->data_size;

    if(pos > 0)
    {
        *key = string_allocate_length(pos+1);
        memcpy((*key)->data, str->data, pos);
    }

    *value = (string_allocate_length(val_end - val_start + 1));
    strcpy((*value)->data, &str->data[val_start]);
}

void string_split_key_value_by_substr(const string_t *str, const char* delimiter, string_t **key, string_t **value)
{
    if(str == NULL || delimiter == NULL)
    {
        return;
    }

    string_t* delimeter_data = string_allocate(delimiter);

    long pos = string_index_of_substr(str, delimeter_data);

    if(pos < 0)
    {
        string_free(&delimeter_data);
        return;
    }

    long val_start = pos + delimeter_data->data_size;
    long val_end = str->data_size;

    if(pos > 0)
    {
        *key = string_allocate_length(pos + 1);
        memcpy((*key)->data, str, pos);
    }

    *value = string_allocate_length(val_end - val_start + 1);
    strcpy((*value)->data, &str->data[val_start]);

    string_free(&delimeter_data);
}

 string_list_t* string_list_allocate_default(void)
 {
    string_list_t* retval = (string_list_t*)calloc(1, sizeof(string_list_t));
    return retval;
 }

string_list_t *string_split_by_substr(const string_t *str, const char *delimiter)
{
	if(str == NULL || delimiter == NULL)
	{
		return NULL;
	}

    string_t* delimeter_data = string_allocate(delimiter);

    long substr_count = string_count_substr(str, delimeter_data);
    long str_len = str->data_size;

	if(substr_count < 1)
	{
        string_free(&delimeter_data);
        return NULL;
	}

    char* ptr = (char*)calloc(1, (unsigned long)str_len+1);

	if(ptr == NULL)
	{
        string_free(&delimeter_data);
        return NULL;
	}

    memcpy(ptr, str->data, (unsigned long)str_len);

    string_list_t* list = (string_list_t*)calloc(1, sizeof(string_list_t));

	char* temp_ptr = NULL;

	temp_ptr = strtok(ptr, delimiter);

	while(temp_ptr != NULL)
	{
        long temp_str_len = (long)strlen(temp_ptr);

        if(temp_str_len < 1)
        {
            continue;
        }

        string_append_to_list(list, temp_ptr);
		temp_ptr = strtok(NULL, delimiter);
	}

    free(ptr);
    string_free(&delimeter_data);

	return list;
}

string_list_t* string_split_by_char(const string_t* str, const char delimiter)
{
	char temp_delimiter[2] = {delimiter, 0};

    return string_split_by_substr(str, temp_delimiter);
}

char* string_join_list_with_substr(const char** strlist, const char* delimiter)
{
    if (strlist == NULL || delimiter == NULL)
    {
        return NULL;
    }

    size_t delimiter_len = strlen(delimiter);
    size_t total_len = 0;
    size_t count = 0;

    // First, calculate total length needed
    while (strlist[count] != NULL)
    {
        total_len += strlen(strlist[count]);
        count++;
    }

    if (count == 0)
    {
        // Empty list
        return NULL;
    }

    // Add space for delimiters between strings
    total_len += delimiter_len * (count - 1);

    // Allocate buffer for result + null terminator
    char* result = (char*)calloc(total_len + 1, sizeof(char));
    if (result == NULL)
    {
        return NULL;
    }

    // Concatenate strings with delimiter
    for (size_t i = 0; i < count; i++)
    {
        strcat(result, strlist[i]);

        if (i < count - 1)
        {
            strcat(result, delimiter);
        }
    }

    return result;
}

char* string_join_list_with_char(const char** strlist, const char delimiter)
{
	char temp_delimiter[2] = { delimiter, 0 };

    return string_join_list_with_substr(strlist, temp_delimiter);
}

char* string_merge_list_with_substr(const char **strlist, const char* delimiter)
{
        if (strlist == NULL || delimiter == NULL)
    {
        return NULL;
    }

    size_t delimiter_len = strlen(delimiter);
    size_t total_len = 0;
    size_t count = 0;

    // Count strings and total length
    while (strlist[count] != NULL)
    {
        total_len += strlen(strlist[count]);
        count++;
    }

    if (count == 0)
    {
        return NULL;
    }

    // Add space for delimiters
    total_len += delimiter_len * (count - 1);

    char* result = (char*)calloc(total_len + 1, sizeof(char));
    if (result == NULL)
    {
        return NULL;
    }

    // Merge strings with delimiter
    for (size_t i = 0; i < count; i++)
    {
        size_t len = strlen(strlist[i]);
        memcpy(result + strlen(result), strlist[i], len);

        if (i < count - 1)
        {
            memcpy(result + strlen(result), delimiter, delimiter_len);
        }
    }

    return result;
}

char* string_merge_list_with_char(const char** strlist, const char delimiter)
{
    if (strlist == NULL)
    {
        return NULL;
    }

    // Calculate total length needed (including delimiters)
    size_t total_length = 0;
    size_t count = 0;

    while (strlist[count] != NULL)
    {
        total_length += strlen(strlist[count]);
        count++;
    }

    if (count == 0)
    {
        // Empty list, return empty string
        char* empty_str = (char*)calloc(1, sizeof(char));
        return empty_str;
    }

    // Add space for delimiters (count - 1) and terminating null
    total_length += (count - 1) + 1;

    char* merged_str = (char*)calloc(total_length, sizeof(char));
    if (merged_str == NULL)
    {
        return NULL;
    }

    size_t pos = 0;
    for (size_t i = 0; i < count; i++)
    {
        size_t len = strlen(strlist[i]);
        memcpy(merged_str + pos, strlist[i], len);
        pos += len;

        // Add delimiter if not the last element
        if (i < count - 1)
        {
            merged_str[pos] = delimiter;
            pos++;
        }
    }

    // Null-terminate (calloc zeroes already, but set explicitly)
    merged_str[pos] = '\0';

    return merged_str;
}

void  string_sort_list(string_list_t *strlist)
{
        if (strlist == NULL)
    {
        return;
    }

    if (strlist->num_of_strings < 2 || strlist->strings == NULL)
    {
        return;
    }

    for (size_t i = 0; i < strlist->num_of_strings - 1; ++i)
    {
        for (size_t j = i + 1; j < strlist->num_of_strings; ++j)
        {
            const char* a = strlist->strings[i].data ? strlist->strings[i].data : "";
            const char* b = strlist->strings[j].data ? strlist->strings[j].data : "";

            if (strcmp(a, b) > 0)
            {
                /* swap the whole string_t structs (shallow swap) */
                string_t tmp = strlist->strings[i];
                strlist->strings[i] = strlist->strings[j];
                strlist->strings[j] = tmp;
            }
        }
    }
}

void string_free_list(string_list_t *strlist)
{
    long index = 0;

    if(strlist != NULL)
    {
        for(int x = 0; x < strlist->num_of_strings; x++)
        {
            if(strlist->strings[x].data != NULL)
            {
                free(strlist->strings[x].data);
                strlist->strings[x].data = NULL;
            }
        }

        free(strlist->strings);
        free(strlist);
    }
}

void  string_append_to_list(string_list_t *strlist, const char *str)
{
    if (strlist == NULL || str == NULL)
    {
        return;
    }

    string_t* new_string = string_allocate(str);

    if (new_string == NULL)
    {
        return;
    }

    // Allocate or reallocate the strings array to hold one more string_t
    if (strlist->strings == NULL)
    {
        strlist->strings = (string_t*)calloc(1, sizeof(string_t));
    }
    else
    {
        string_t* temp = (string_t*)realloc(strlist->strings, (strlist->num_of_strings + 1) * sizeof(string_t));
        if (temp == NULL)
        {
            string_free(&new_string);
            return;
        }
        strlist->strings = temp;
    }

    // Copy the new string_t struct contents into the array at the next position
    strlist->strings[strlist->num_of_strings] = *new_string;

    // Free the temporary pointer but NOT the internal data (which was assigned)
    free(new_string);

    strlist->num_of_strings++;
}

void  string_append_string_to_list(string_list_t* strlist, const string_t* str)
{
    if (strlist == NULL || str == NULL)
    {
        return;
    }

    // Append the internal data of string_t 'str' to the list using existing function
    string_append_to_list(strlist, str->data);
}

string_t* string_get_first_from_list(string_list_t* strlist)
{
    if(strlist != NULL)
    {
        if(strlist->num_of_strings > 0)
        {
            strlist->current_index = 0;
            return &strlist->strings[0];
        }
    }

    return NULL;
}

string_t *string_get_next_from_list(string_list_t *strlist)
{
    if(strlist != NULL)
    {
        strlist->current_index++;
        if(strlist->num_of_strings > 1 && strlist->current_index < strlist->num_of_strings)
        {
            return &strlist->strings[strlist->current_index];
        }
    }

    return NULL;
}


///////////////////////////////////////////////////////////

string_t* string_internal_adjust_storage(string_t* string_ptr, size_t sz)
{
    if(string_ptr == NULL)
    {
        return NULL;
    }

    size_t psize = sysconf(_SC_PAGESIZE);
    size_t required_size = string_ptr->data_size + sz;
    
    // Calculate pages needed for required size
    size_t pcount = required_size / psize;
    if(required_size % psize != 0)
    {
        pcount++;
    }
    
    size_t new_memory_size = psize * pcount;
    
    // Expand if needed
    if(new_memory_size > string_ptr->memory_size)
    {
        void* ptr = (char*)calloc(new_memory_size, sizeof(char));
        if(ptr)
        {
            // Copy existing data
            for(size_t i = 0; i < string_ptr->data_size; i++)
            {
                ((char*)ptr)[i] = string_ptr->data[i];
            }
            
            free(string_ptr->data);
            string_ptr->data = ptr;
            string_ptr->memory_size = new_memory_size;
        }
    }
    // Shrink if wasting more than 2 pages
    else 
    {
        if(string_ptr->memory_size > new_memory_size + (2 * psize))
        {
            void* ptr = (char*)calloc(new_memory_size, sizeof(char));
            if(ptr)
            {
                // Copy existing data
                for(size_t i = 0; i < string_ptr->data_size; i++)
                {
                    ((char*)ptr)[i] = string_ptr->data[i];
                }
                
                free(string_ptr->data);
                string_ptr->data = ptr;
                string_ptr->memory_size = new_memory_size;
            }
        }
    }

    return string_ptr;
}

char* string_internal_from_int(long num)
{
    char* ptr = (char*)calloc(1, (unsigned long)32);

    if (ptr == NULL)
    {
        return NULL;
    }

    int sign = 1;
    long remainder = 1;
    long dividend = num;
    long ctr = 0;

    if (num < 1)
    {
        sign = -1;
        dividend = dividend*(long)(-1);
    }

    while (dividend && ctr < 32)
    {
        remainder = dividend % 10;
        dividend = dividend / 10;

        ptr[ctr] = (char)(remainder + 48);
        ctr++;
    }

    if (sign < 1)
    {
        ptr[ctr] = '-';
    }
    else
    {
        ctr--;
    }

    long start = 0;

    while (start < ctr)
    {
        char temp = ptr[start];
        ptr[start] = ptr[ctr];
        ptr[ctr] = temp;
        start++;
        ctr--;
    }

    return ptr;
}

char* string_internal_from_double(double num)
{
    char* ptr = (char*)calloc(1, 64);  // Enough buffer for sign, digits, decimal point, fraction
    if(ptr == NULL)
    {
        return NULL;
    }

    long integral_part = (long)num;
    double fractional_part = num - (double)integral_part;
    if (fractional_part < 0)
    {
        fractional_part = -fractional_part;
    }

    // Convert integral part to string using existing int conversion logic
    char* int_str = string_internal_from_int(integral_part);
    if(int_str == NULL)
    {
        free(ptr);
        return NULL;
    }

    // Copy integral part to ptr
    int pos = 0;
    for(int i = 0; int_str[i] != '\0'; i++)
    {
        ptr[pos++] = int_str[i];
    }
    free(int_str);

    ptr[pos++] = '.';  // decimal point

    // Convert fractional part, 6 digits precision
    for(int i = 0; i < 6; i++)
    {
        fractional_part *= 10.0;
        int digit = (int)fractional_part;
        ptr[pos++] = (char)(digit + '0');
        fractional_part -= digit;
    }

    ptr[pos] = '\0';

    return ptr;
}

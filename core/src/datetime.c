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

#include "datetime.h"
#include <memory.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>

typedef struct date_time_t
{
    struct tm timeinfo;

}date_time_t;

char* date_time_get_default_string(char* ptr)
{
    if(!ptr)
    {
        return  NULL;
    }

    char buffer[65] = {0};

    time_t t ;
    struct tm *tmp ;
    time(&t);
    tmp = localtime(&t);

	snprintf(buffer, sizeof(buffer), "%04d%02d%02d%02d%02d%02d",
             (tmp->tm_year+1900), (tmp->tm_mon+1),tmp->tm_mday,
             tmp->tm_hour, tmp->tm_min, tmp->tm_sec);

    strcpy(ptr, buffer);

    return ptr;
}

void date_time_release(date_time_t* ptr)
{
	if(!ptr)
    {
        return;
    }

	free(ptr);
	ptr = NULL;
}

date_time_t* date_time_allocate_default()
{
	date_time_t* retval = (date_time_t*)calloc(1, sizeof(date_time_t));

	if(retval == NULL)
	{
		return NULL;
	}

	time_t t ;
    struct tm *tmp ;
    time(&t);
    tmp = localtime(&t);

	if(tmp == NULL)
	{
		free(retval);
		return NULL;
	}

	retval->timeinfo = *tmp;

	return retval;
}

// Helper function to extract integer from string at position
static int extract_int_from_string(const char* str, size_t pos, int len)
{
	if(str == NULL || pos + len > strlen(str)) 
	{
		return 0;
	}
	
	char temp[5] = {0};
	strncpy(temp, str + pos, len);
	return atoi(temp);
}

date_time_t* date_time_allocate_from_string(const char* strts, const char* strformat)
{
	if(strts == NULL || strformat == NULL)
	{
		return NULL;
	}

	date_time_t* retval = (date_time_t*)calloc(1, sizeof(date_time_t));
	if(retval == NULL)
	{
		return NULL;
	}

	// Initialize with current time as base
	time_t t;
	time(&t);
	struct tm* tmp = localtime(&t);
	retval->timeinfo = *tmp;

	// Simple string parsing based on your C++ logic
	size_t str_len = strlen(strts);

	if(str_len != 14 && str_len != 12)
	{
		free(retval);
		return NULL;
	}

	// Find and parse year
	char* pos = strstr(strformat, "yyyy");
	if(pos != NULL)
	{
		size_t idx = pos - strformat;
		retval->timeinfo.tm_year = extract_int_from_string(strts, idx, 4) - 1900;
	}
	else
	{
		pos = strstr(strformat, "yy");
		if(pos != NULL)
		{
			size_t idx = pos - strformat;
			retval->timeinfo.tm_year = extract_int_from_string(strts, idx, 2) + 100;
		}
	}

	// Find and parse month
	pos = strstr(strformat, "MM");
	if(pos != NULL)
	{
		size_t idx = pos - strformat;
		retval->timeinfo.tm_mon = extract_int_from_string(strts, idx, 2) - 1;
	}

	// Find and parse day
	pos = strstr(strformat, "dd");
	if(pos != NULL)
	{
		size_t idx = pos - strformat;
		retval->timeinfo.tm_mday = extract_int_from_string(strts, idx, 2);
	}

	// Find and parse hour
	pos = strstr(strformat, "hh");
	if(pos != NULL)
	{
		size_t idx = pos - strformat;
		retval->timeinfo.tm_hour = extract_int_from_string(strts, idx, 2);
	}

	// Find and parse minute  
	pos = strstr(strformat, "mm");
	if(pos != NULL)
	{
		size_t idx = pos - strformat;
		retval->timeinfo.tm_min = extract_int_from_string(strts, idx, 2);
	}

	// Find and parse second
	pos = strstr(strformat, "ss");
	if(pos != NULL)
	{
		size_t idx = pos - strformat;
		retval->timeinfo.tm_sec = extract_int_from_string(strts, idx, 2);
	}

	return retval;
}

// FIX 1: Critical bug - was calling time(&unixtsval) instead of using the parameter
date_time_t* date_time_allocate_from_unix_epoch(const time_t unixtsval)
{
	date_time_t* retval = (date_time_t*)calloc(1, sizeof(date_time_t));

	if(retval == NULL)
	{
		return NULL;
	}

	struct tm *tmp ;
    tmp = localtime(&unixtsval);
    if(tmp == NULL)
    {
        free(retval);
        return NULL;
    }
	retval->timeinfo = *tmp;

	return retval;
}

// FIX 2: Change parameter type to time_t for consistency
date_time_t* date_time_allocate_from_time(const time_t tsval)
{
	return date_time_allocate_from_unix_epoch(tsval);
}

date_time_t* date_time_allocate_from_time_struct(const struct tm* tsval)
{
	if(tsval == NULL)
	{
		return NULL;
	}

	date_time_t* retval = (date_time_t*)calloc(1, sizeof(date_time_t));

	if(retval == NULL)
	{
		return NULL;
	}

	retval->timeinfo = *tsval;

	return retval;
}

// FIX 3: Document that caller must free returned string
// NOTE: Caller must call free() on returned string
char* date_time_get_string(date_time_t* ptr)
{
	char* retval = NULL;

	if(ptr == NULL)
	{
		return NULL;
	}

	retval = (char*)calloc(1, 65);

	if(retval == NULL)
	{			
		return NULL;
	}

	sprintf(retval, "%04d%02d%02d%02d%02d%02d",
			(ptr->timeinfo.tm_year+1900), (ptr->timeinfo.tm_mon+1), ptr->timeinfo.tm_mday,
			ptr->timeinfo.tm_hour, ptr->timeinfo.tm_min, ptr->timeinfo.tm_sec);

	return retval;
}

// Helper function to replace substring in string (for formatting)
static void str_replace_format(char* dest, const char* src, const char* find, const char* replace, size_t dest_size)
{
	char* pos = strstr(src, find);
	if(pos == NULL)
	{
		strncpy(dest, src, dest_size - 1);
		dest[dest_size - 1] = '\0';
		return;
	}
	
	size_t prefix_len = pos - src;
	size_t find_len = strlen(find);
	size_t replace_len = strlen(replace);
	size_t suffix_len = strlen(pos + find_len);
	
	if(prefix_len + replace_len + suffix_len >= dest_size)
	{
		strncpy(dest, src, dest_size - 1);
		dest[dest_size - 1] = '\0';
		return;
	}
	
	strncpy(dest, src, prefix_len);
	strcpy(dest + prefix_len, replace);
	strcpy(dest + prefix_len + replace_len, pos + find_len);
}

// NOTE: Caller must call free() on returned string
char* date_time_get_formatted_string(date_time_t* ptr, const char* strformat)
{
	if(ptr == NULL || strformat == NULL)
	{
		return NULL;
	}

	char* retval = (char*)calloc(1, 256);
	if(retval == NULL)
	{
		return NULL;
	}

	char temp_format[256] = {0};
	strncpy(temp_format, strformat, 255);
	
	bool am_pm_needed = false;
	
	// Convert custom format to strftime format
	char work_buf[256] = {0};
	
	// Handle seconds
	if(strstr(temp_format, "ss"))
	{
		str_replace_format(work_buf, temp_format, "ss", "%S", sizeof(work_buf));
		strcpy(temp_format, work_buf);
	}
	
	// Handle minutes
	if(strstr(temp_format, "mm"))
	{
		str_replace_format(work_buf, temp_format, "mm", "%M", sizeof(work_buf));
		strcpy(temp_format, work_buf);
	}
	
	// Handle hours
	if(strstr(temp_format, "hh"))
	{
		str_replace_format(work_buf, temp_format, "hh", "%H", sizeof(work_buf));
		strcpy(temp_format, work_buf);
	}
	else if(strstr(temp_format, "h"))
	{
		str_replace_format(work_buf, temp_format, "h", "%I", sizeof(work_buf));
		strcpy(temp_format, work_buf);
		am_pm_needed = true;
	}
	
	// Handle day
	if(strstr(temp_format, "dd"))
	{
		str_replace_format(work_buf, temp_format, "dd", "%d", sizeof(work_buf));
		strcpy(temp_format, work_buf);
	}
	
	// Handle month name vs number
	if(strstr(temp_format, "MMMM"))
	{
		str_replace_format(work_buf, temp_format, "MMMM", "%B", sizeof(work_buf));
		strcpy(temp_format, work_buf);
	}
	else if(strstr(temp_format, "MM"))
	{
		str_replace_format(work_buf, temp_format, "MM", "%m", sizeof(work_buf));
		strcpy(temp_format, work_buf);
	}
	
	// Handle year
	if(strstr(temp_format, "yyyy"))
	{
		str_replace_format(work_buf, temp_format, "yyyy", "%Y", sizeof(work_buf));
		strcpy(temp_format, work_buf);
	}
	else if(strstr(temp_format, "yy"))
	{
		str_replace_format(work_buf, temp_format, "yy", "%y", sizeof(work_buf));
		strcpy(temp_format, work_buf);
	}
	
	// Add AM/PM if needed
	if(am_pm_needed)
	{
		size_t temp_len = strlen(temp_format);
		const char* ampm = " %p";
		size_t ampm_len = strlen(ampm);

		if(temp_len + ampm_len >= sizeof(temp_format))
		{
			free(retval);
			return NULL;
		}

		memcpy(temp_format + temp_len, ampm, ampm_len + 1);
	}
	
	// Make a copy of timeinfo to avoid modification
	struct tm temp_tm = ptr->timeinfo;
	
	// Ensure year is correct for strftime
	if(temp_tm.tm_year < 100)
	{
		temp_tm.tm_year += 100;
	}
	
	// Format the string
	if(strftime(retval, 256, temp_format, &temp_tm) == 0)
	{
		free(retval);
		return NULL;
	}

	return retval;
}

time_t date_time_get_unix_epoch(date_time_t* ptr)
{
	if(ptr == NULL)
	{
		return 0;
	}
	
	// Make a copy to avoid modifying original
	struct tm temp_tm = ptr->timeinfo;
	return mktime(&temp_tm);
}

time_t date_time_get_time(date_time_t* ptr)
{
	return date_time_get_unix_epoch(ptr);
}

date_time_t* date_time_add_days(date_time_t* ptr, unsigned long val)
{
	if(ptr == NULL)
	{
		return NULL;
	}
	
	// Create a copy to avoid modifying original
	date_time_t* retval = (date_time_t*)calloc(1, sizeof(date_time_t));
	if(retval == NULL)
	{
		return NULL;
	}
	
	retval->timeinfo = ptr->timeinfo;
	
	// Add seconds equivalent of days and let mktime normalize
	retval->timeinfo.tm_sec += (val * 24 * 60 * 60);
	
	// Normalize the time structure
	mktime(&retval->timeinfo);
	
	return retval;
}

date_time_t* date_time_add_hours(date_time_t* ptr, unsigned long val)
{
	if(ptr == NULL)
	{
		return NULL;
	}
	
	// Create a copy to avoid modifying original
	date_time_t* retval = (date_time_t*)calloc(1, sizeof(date_time_t));
	if(retval == NULL)
	{
		return NULL;
	}
	
	retval->timeinfo = ptr->timeinfo;
	
	// Add seconds equivalent of hours and let mktime normalize
	retval->timeinfo.tm_sec += (val * 60 * 60);
	
	// Normalize the time structure
	mktime(&retval->timeinfo);
	
	return retval;
}

date_time_t* date_time_add_minutes(date_time_t* ptr, unsigned long val)
{
	if(ptr == NULL)
	{
		return NULL;
	}
	
	// Create a copy to avoid modifying original
	date_time_t* retval = (date_time_t*)calloc(1, sizeof(date_time_t));
	if(retval == NULL)
	{
		return NULL;
	}
	
	retval->timeinfo = ptr->timeinfo;
	
	// Add seconds equivalent of minutes and let mktime normalize
	retval->timeinfo.tm_sec += (val * 60);
	
	// Normalize the time structure
	mktime(&retval->timeinfo);
	
	return retval;
}

date_time_t* date_time_add_seconds(date_time_t* ptr, unsigned long val)
{
	if(ptr == NULL)
	{
		return NULL;
	}
	
	// Create a copy to avoid modifying original
	date_time_t* retval = (date_time_t*)calloc(1, sizeof(date_time_t));
	if(retval == NULL)
	{
		return NULL;
	}
	
	retval->timeinfo = ptr->timeinfo;
	
	// Add seconds and let mktime normalize
	retval->timeinfo.tm_sec += val;
	
	// Normalize the time structure
	mktime(&retval->timeinfo);
	
	return retval;
}

unsigned long date_time_get_hours(date_time_t* ptr)
{
	if(ptr == NULL)
	{
		return 0;
	}

	return ptr->timeinfo.tm_hour;
}

unsigned long date_time_get_minutes(date_time_t* ptr)
{
	if(ptr == NULL)
	{
		return 0;
	}

	return ptr->timeinfo.tm_min;
}

unsigned long date_time_get_seconds(date_time_t* ptr)
{
	if(ptr == NULL)
	{
		return 0;
	}

	return ptr->timeinfo.tm_sec;
}

// FIX 4: Milliseconds function was returning seconds - struct tm doesn't have milliseconds
// Return 0 since struct tm doesn't support millisecond precision
unsigned long date_time_get_miliseconds(date_time_t* ptr)
{
	if(ptr == NULL)
	{
		return 0;
	}

	// struct tm doesn't have millisecond precision, always return 0
	return 0;
}

unsigned long date_time_get_year(date_time_t* ptr)
{
	if(ptr == NULL)
	{
		return 0;
	}

	return ptr->timeinfo.tm_year+1900;
}

unsigned long date_time_get_month(date_time_t* ptr)
{
	if(ptr == NULL)
	{
		return 0;
	}

	return ptr->timeinfo.tm_mon+1;
}

// FIX 5: Clarify function purpose - tm_yday is day of year (1-366)
unsigned long date_time_get_days(date_time_t* ptr)
{
	if(ptr == NULL)
	{
		return 0;
	}

	return ptr->timeinfo.tm_yday + 1; // tm_yday is 0-based, make it 1-based
}

unsigned long date_time_get_day_of_month(date_time_t* ptr)
{
	if(ptr == NULL)
	{
		return 0;
	}

	return ptr->timeinfo.tm_mday;
}

unsigned long date_time_get_day_of_week(date_time_t* ptr)
{
	if(ptr == NULL)
	{
		return 0;
	}

	return ptr->timeinfo.tm_wday;
}

unsigned long date_time_get_week_of_year(date_time_t* ptr)
{
	if(ptr == NULL)
	{
		return 0;
	}
	
	// Make a copy to avoid modifying original
	struct tm temp_tm = ptr->timeinfo;
	
	// Normalize to ensure tm_yday is calculated
	mktime(&temp_tm);
	
	// Simple week calculation: day of year / 7 + 1
	// This is approximate - proper ISO week calculation is more complex
	return (temp_tm.tm_yday / 7) + 1;
}

// FIX 5: Add basic bounds checking for setters
void date_time_set_year(date_time_t* ptr, unsigned long val)
{
	if(ptr == NULL || val < 1900 || val > 3000)
	{
		return;
	}
	
	ptr->timeinfo.tm_year = val - 1900;
}

void date_time_set_month(date_time_t* ptr, unsigned long val)
{
	if(ptr == NULL || val < 1 || val > 12)
	{
		return;
	}
	
	ptr->timeinfo.tm_mon = val - 1;
}

void date_time_set_day(date_time_t* ptr, unsigned long val)
{
	if(ptr == NULL || val < 1 || val > 31)
	{
		return;
	}
	
	ptr->timeinfo.tm_mday = val;
}

void date_time_set_hour(date_time_t* ptr, unsigned long val)
{
	if(ptr == NULL || val > 23)
	{
		return;
	}
	
	ptr->timeinfo.tm_hour = val;
}

void date_time_set_minute(date_time_t* ptr, unsigned long val)
{
	if(ptr == NULL || val > 59)
	{
		return;
	}
	
	ptr->timeinfo.tm_min = val;
}

void date_time_set_second(date_time_t* ptr, unsigned long val)
{
	if(ptr == NULL || val > 59)
	{
		return;
	}
	
	ptr->timeinfo.tm_sec = val;
}

bool date_time_are_equal(date_time_t* first, date_time_t* second)
{
	if(first == NULL || second == NULL)
	{
		return false;
	}
	
	// Convert both to time_t for comparison
	struct tm temp1 = first->timeinfo;
	struct tm temp2 = second->timeinfo;
	
	time_t t1 = mktime(&temp1);
	time_t t2 = mktime(&temp2);
	
	return (t1 == t2);
}

bool date_time_is_first_greater(date_time_t* first, date_time_t* second)
{
	if(first == NULL || second == NULL)
	{
		return false;
	}
	
	// Convert both to time_t for comparison
	struct tm temp1 = first->timeinfo;
	struct tm temp2 = second->timeinfo;
	
	time_t t1 = mktime(&temp1);
	time_t t2 = mktime(&temp2);
	
	return (t1 > t2);
}

bool date_time_is_first_lower(date_time_t* first, date_time_t* second)
{
	if(first == NULL || second == NULL)
	{
		return false;
	}
	
	// Convert both to time_t for comparison
	struct tm temp1 = first->timeinfo;
	struct tm temp2 = second->timeinfo;
	
	time_t t1 = mktime(&temp1);
	time_t t2 = mktime(&temp2);
	
	return (t1 < t2);
}

unsigned long date_time_diff_miliseconds(date_time_t* first, date_time_t* second)
{
	if(first == NULL || second == NULL)
	{
		return 0;
	}
	
	// Convert both to time_t for comparison
	struct tm temp1 = first->timeinfo;
	struct tm temp2 = second->timeinfo;
	
	time_t t1 = mktime(&temp1);
	time_t t2 = mktime(&temp2);
	
	// Calculate difference in seconds, then convert to milliseconds
	double diff_seconds = difftime(t1, t2);
	
	// Since we don't have sub-second precision, multiply by 1000
	// and return absolute value
	return (unsigned long)(diff_seconds < 0 ? -diff_seconds * 1000 : diff_seconds * 1000);
}

unsigned long long date_time_to_epoch_ll(date_time_t* ptr)
{
	if(ptr == NULL)
	{
		return 0;
	}
	
	// Use existing time_t function and convert
	time_t t = date_time_get_unix_epoch(ptr);
	
	// Handle potential negative time_t (dates before 1970)
	if(t < 0)
	{
		return 0;  // Or handle as appropriate for your use case
	}
	
	return (unsigned long long)t;

}

date_time_t* date_time_from_epoch_ll(unsigned long long epoch)
{
	// Check for reasonable epoch range to avoid overflow
	// time_t on 32-bit systems typically maxes out at 2147483647 (2038-01-19)
	// But since we're converting TO time_t, let the system handle the limits
	
	time_t t = (time_t)epoch;
	
	// Sanity check: if the conversion truncated the value, it's out of range
	if((unsigned long long)t != epoch)
	{
		// Epoch value too large for this system's time_t
		return NULL;
	}
	
	// Use existing time_t function
	return date_time_allocate_from_unix_epoch(t);
}

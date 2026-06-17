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

#ifndef DATE_TIME
#define DATE_TIME

#include "defines.h"
#include <string.h>
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct date_time_t date_time_t;

extern LIBRARY_EXPORT char* date_time_get_default_string(char* ptr);
extern LIBRARY_EXPORT void date_time_release(date_time_t* ptr);
extern LIBRARY_EXPORT date_time_t* date_time_allocate_default();
extern LIBRARY_EXPORT date_time_t* date_time_allocate_from_string(const char* strts, const char* strformat);
extern LIBRARY_EXPORT date_time_t* date_time_allocate_from_unix_epoch(const time_t unixtsval);
extern LIBRARY_EXPORT date_time_t* date_time_allocate_from_time(const time_t tsval);
extern LIBRARY_EXPORT date_time_t* date_time_allocate_from_time_struct(const struct tm* tsval);
extern LIBRARY_EXPORT char* date_time_get_string(date_time_t* ptr);
extern LIBRARY_EXPORT char* date_time_get_formatted_string(date_time_t* ptr, const char* strformat);
extern LIBRARY_EXPORT time_t date_time_get_unix_epoch(date_time_t* ptr);
extern LIBRARY_EXPORT time_t date_time_get_time(date_time_t* ptr);

extern LIBRARY_EXPORT date_time_t* date_time_add_days(date_time_t* ptr, unsigned long val);
extern LIBRARY_EXPORT date_time_t* date_time_add_hours(date_time_t* ptr, unsigned long val);
extern LIBRARY_EXPORT date_time_t* date_time_add_minutes(date_time_t* ptr, unsigned long val);
extern LIBRARY_EXPORT date_time_t* date_time_add_seconds(date_time_t* ptr, unsigned long val);

extern LIBRARY_EXPORT unsigned long date_time_get_hours(date_time_t* ptr);
extern LIBRARY_EXPORT unsigned long date_time_get_minutes(date_time_t* ptr);
extern LIBRARY_EXPORT unsigned long date_time_get_seconds(date_time_t* ptr);
extern LIBRARY_EXPORT unsigned long date_time_get_miliseconds(date_time_t* ptr);

extern LIBRARY_EXPORT unsigned long date_time_get_year(date_time_t* ptr);
extern LIBRARY_EXPORT unsigned long date_time_get_month(date_time_t* ptr);
extern LIBRARY_EXPORT unsigned long date_time_get_days(date_time_t* ptr);
extern LIBRARY_EXPORT unsigned long date_time_get_day_of_month(date_time_t* ptr);
extern LIBRARY_EXPORT unsigned long date_time_get_day_of_week(date_time_t* ptr);
extern LIBRARY_EXPORT unsigned long date_time_get_week_of_year(date_time_t* ptr);

extern LIBRARY_EXPORT void date_time_set_year(date_time_t* ptr, unsigned long val);
extern LIBRARY_EXPORT void date_time_set_month(date_time_t* ptr, unsigned long val);
extern LIBRARY_EXPORT void date_time_set_day(date_time_t* ptr, unsigned long val);
extern LIBRARY_EXPORT void date_time_set_hour(date_time_t* ptr, unsigned long val);
extern LIBRARY_EXPORT void date_time_set_minute(date_time_t* ptr, unsigned long val);
extern LIBRARY_EXPORT void date_time_set_second(date_time_t* ptr, unsigned long val);

extern LIBRARY_EXPORT bool date_time_are_equal(date_time_t* first, date_time_t* second);
extern LIBRARY_EXPORT bool date_time_is_first_greater(date_time_t* first, date_time_t* second);
extern LIBRARY_EXPORT bool date_time_is_first_lower(date_time_t* first, date_time_t* second);

extern LIBRARY_EXPORT unsigned long date_time_diff_miliseconds(date_time_t* first, date_time_t* second);

// Handrail functions for external epoch timestamps
extern LIBRARY_EXPORT unsigned long long date_time_to_epoch_ll(date_time_t* ptr);
extern LIBRARY_EXPORT date_time_t* date_time_from_epoch_ll(unsigned long long epoch);

#ifdef __cplusplus
}
#endif

#endif


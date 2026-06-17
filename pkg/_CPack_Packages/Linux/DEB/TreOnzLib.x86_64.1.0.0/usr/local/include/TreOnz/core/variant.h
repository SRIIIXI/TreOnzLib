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

#ifndef VARIANT_C
#define VARIANT_C

#include "defines.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum VariantType
{
    Void = 0,
    Char = 1,
    UnsignedChar = 2,
    String = 3,
    Boolean = 4,
    Number = 5,
    UnsignedNumber = 6,
    Decimal = 7,
    DateTimeStamp = 8,
    Raw = 9
}VariantType;

typedef struct variant_t variant_t;

extern LIBRARY_EXPORT variant_t* variant_allocate_default();
extern LIBRARY_EXPORT void variant_release(variant_t* varptr);
extern LIBRARY_EXPORT variant_t* variant_allocate(variant_t* varptr);
extern LIBRARY_EXPORT variant_t* variant_allocate_char(char ch);
extern LIBRARY_EXPORT variant_t* variant_allocate_unsigned_char(unsigned char ch);
extern LIBRARY_EXPORT variant_t* variant_allocate_string(const char* str, size_t ln);
extern LIBRARY_EXPORT variant_t* variant_allocate_bool(bool fl);
extern LIBRARY_EXPORT variant_t* variant_allocate_long(long val);
extern LIBRARY_EXPORT variant_t* variant_allocate_unsigned_long(unsigned long val);
extern LIBRARY_EXPORT variant_t* variant_allocate_double(double val);
extern LIBRARY_EXPORT variant_t* variant_allocate_time_value(unsigned long val);

extern LIBRARY_EXPORT void variant_set_variant(variant_t* varptr, variant_t* val);
extern LIBRARY_EXPORT void variant_set_char(variant_t* varptr, char ch);
extern LIBRARY_EXPORT void variant_set_unsigned_char(variant_t* varptr, unsigned char ch);
extern LIBRARY_EXPORT void variant_set_string(variant_t* varptr, const char* str, size_t ln);
extern LIBRARY_EXPORT void variant_set_bool(variant_t* varptr, bool fl);
extern LIBRARY_EXPORT void variant_set_long(variant_t* varptr, long val);
extern LIBRARY_EXPORT void variant_set_unsigned_long(variant_t* varptr, unsigned long val);
extern LIBRARY_EXPORT void variant_set_double(variant_t* varptr, double val);
extern LIBRARY_EXPORT void variant_set_time_value(variant_t* varptr, unsigned long val);

extern LIBRARY_EXPORT VariantType variant_get_data_type(variant_t* varptr);
extern LIBRARY_EXPORT size_t variant_get_data_size(variant_t* varptr);

extern LIBRARY_EXPORT char variant_get_char(variant_t* varptr);
extern LIBRARY_EXPORT unsigned char variant_get_unsigned_char(variant_t* varptr);
extern LIBRARY_EXPORT const char* variant_get_string(variant_t* varptr);
extern LIBRARY_EXPORT bool variant_get_bool(variant_t* varptr);
extern LIBRARY_EXPORT long variant_get_long(variant_t* varptr);
extern LIBRARY_EXPORT unsigned long variant_get_unsigned_long(variant_t* varptr);
extern LIBRARY_EXPORT double variant_get_double(variant_t* varptr);
extern LIBRARY_EXPORT unsigned long variant_get_time_value(variant_t* varptr);

#ifdef __cplusplus
}
#endif

#endif


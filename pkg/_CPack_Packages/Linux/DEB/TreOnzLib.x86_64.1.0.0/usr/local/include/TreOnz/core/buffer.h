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

#ifndef BUFFER_C
#define BUFFER_C

#include "defines.h"
#include "stringex.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct buffer_t buffer_t;

extern LIBRARY_EXPORT buffer_t* buffer_allocate(const void* data, size_t sz);
extern LIBRARY_EXPORT buffer_t* buffer_allocate_default(void);
extern LIBRARY_EXPORT buffer_t* buffer_allocate_length(size_t len);

extern LIBRARY_EXPORT buffer_t* buffer_copy(buffer_t* dest, buffer_t* orig);
extern LIBRARY_EXPORT buffer_t* buffer_append(buffer_t* dest, const void* data, size_t sz);

extern LIBRARY_EXPORT void buffer_remove(buffer_t* ptr, size_t start, size_t len);
extern LIBRARY_EXPORT void buffer_remove_end(buffer_t* ptr, size_t len);
extern LIBRARY_EXPORT void buffer_remove_start(buffer_t* ptr, size_t len);

extern LIBRARY_EXPORT void buffer_clear(buffer_t* ptr);
extern LIBRARY_EXPORT void buffer_free(buffer_t** ptr);

extern LIBRARY_EXPORT bool buffer_is_equal(buffer_t* first, buffer_t* second);
extern LIBRARY_EXPORT bool buffer_is_greater(buffer_t* first, buffer_t* second);
extern LIBRARY_EXPORT bool buffer_is_less(buffer_t* first, buffer_t* second);
extern LIBRARY_EXPORT bool buffer_is_null(buffer_t* ptr);

extern LIBRARY_EXPORT const void* buffer_get_data(const buffer_t* ptr);
extern LIBRARY_EXPORT size_t buffer_get_size(const buffer_t* ptr);
extern LIBRARY_EXPORT string_t* buffer_convert_to_string(buffer_t* ptr);


#ifdef __cplusplus
}
#endif

#endif

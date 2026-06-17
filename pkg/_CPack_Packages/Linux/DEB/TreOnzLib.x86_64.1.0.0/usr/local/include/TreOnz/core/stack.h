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

#ifndef STACK_C
#define STACK_C

#include "defines.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct stack_t stack_t;

extern LIBRARY_EXPORT stack_t* stack_allocate(stack_t* sptr);
extern LIBRARY_EXPORT void stack_clear(stack_t* sptr);
extern LIBRARY_EXPORT void stack_free(stack_t* sptr);

extern LIBRARY_EXPORT void stack_push(stack_t* sptr, void* data, size_t sz);
extern LIBRARY_EXPORT void* stack_pop(stack_t* sptr, size_t* out_size);
extern LIBRARY_EXPORT long stack_item_count(stack_t* sptr);
extern LIBRARY_EXPORT void* stack_peek(stack_t* sptr, size_t* out_size);

#ifdef __cplusplus
}
#endif

#endif


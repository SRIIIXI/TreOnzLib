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

#ifndef CONFIGURATION_C
#define CONFIGURATION_C

#include "defines.h"
#include "stringex.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct configuration_t configuration_t;

extern LIBRARY_EXPORT configuration_t* configuration_allocate_default(void);
extern LIBRARY_EXPORT configuration_t* configuration_allocate(const char* filename);
extern LIBRARY_EXPORT void  configuration_release(configuration_t* config);

extern LIBRARY_EXPORT string_list_t*  configuration_get_all_sections(const configuration_t* config);
extern LIBRARY_EXPORT string_list_t*  configuration_get_all_keys(const configuration_t* config, const char* section);

extern LIBRARY_EXPORT bool  configuration_has_section(const configuration_t* config, const char* section);
extern LIBRARY_EXPORT bool  configuration_has_key(const configuration_t* config, const char* section, char* key);

extern LIBRARY_EXPORT long  configuration_get_value_as_integer(const configuration_t* config, const char* section, const char* key);
extern LIBRARY_EXPORT bool  configuration_get_value_as_boolean(const configuration_t* config, const char* section, const char* key);
extern LIBRARY_EXPORT double configuration_get_value_as_real(const configuration_t* config, const char* section, const char* key);
extern LIBRARY_EXPORT const char *configuration_get_value_as_string(const configuration_t* config, const char* section, const char* key);
extern LIBRARY_EXPORT char configuration_get_value_as_char(const configuration_t* config, const char* section, const char* key);

#ifdef __cplusplus
}
#endif

#endif

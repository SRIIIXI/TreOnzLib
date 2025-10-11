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

#ifndef ABNF_C
#define ABNF_C

#include "defines.h"
#include "buffer.h"
#include "stringex.h"
#include "mime.h"

#ifdef __cplusplus
extern "C"
{
#endif

    typedef enum abnf_protcol_usage_t
    {
        ABNF_PROTOCOL_USAGE_HTTP = 0,
        ABNF_PROTOCOL_USAGE_SIP = 1,
        ABNF_PROTOCOL_USAGE_SMTP = 2,
        ABNF_PROTOCOL_USAGE_IMAP4 = 3,
        ABNF_PROTOCOL_USAGE_UNKNOWN = 4
    }abnf_protcol_usage_t;

    typedef struct abnf_t abnf_t;

    extern LIBRARY_EXPORT abnf_t *abnf_allocate();
    extern LIBRARY_EXPORT abnf_t *abnf_parse_and_allocate(string_t *data);
    extern LIBRARY_EXPORT abnf_t *abnf_parse_and_allocate_from_headers(string_t *data);
    extern LIBRARY_EXPORT void abnf_free(abnf_t **ptr);
    extern LIBRARY_EXPORT bool abnf_is_valid(abnf_t *ptr); 
    extern LIBRARY_EXPORT string_t* abnf_serialize(abnf_t *ptr, abnf_protcol_usage_t usage);

    //Getters
    extern LIBRARY_EXPORT abnf_protcol_usage_t abnf_get_protocol(abnf_t *ptr);
    extern LIBRARY_EXPORT string_t* abnf_get_protocol_version(abnf_t *ptr);
    extern LIBRARY_EXPORT string_t* abnf_get_method(abnf_t *ptr);
    extern LIBRARY_EXPORT string_t* abnf_get_request_uri(abnf_t *ptr);
    extern LIBRARY_EXPORT int abnf_get_status_code(abnf_t *ptr);
    extern LIBRARY_EXPORT string_t* abnf_get_reason_phrase(abnf_t *ptr);
    extern LIBRARY_EXPORT string_t* abnf_get_header_value(abnf_t *ptr, const char* header_name);
    extern LIBRARY_EXPORT string_list_t* abnf_get_all_headers(abnf_t *ptr);
    extern LIBRARY_EXPORT string_t* abnf_get_body(abnf_t *ptr);

    //Setters
    extern LIBRARY_EXPORT bool abnf_set_body(abnf_t *ptr, const char* body);
    extern LIBRARY_EXPORT bool abnf_set_header_value(abnf_t *ptr, const char* header_name, const char* header_value);
    extern LIBRARY_EXPORT bool abnf_set_method(abnf_t *ptr, const char* method);
    extern LIBRARY_EXPORT bool abnf_set_request_uri(abnf_t *ptr, const char* uri);
    extern LIBRARY_EXPORT bool abnf_set_status_code(abnf_t *ptr, int status_code);
    extern LIBRARY_EXPORT bool abnf_set_reason_phrase(abnf_t *ptr, const char* reason_phrase);
    extern LIBRARY_EXPORT bool abnf_set_protocol_version(abnf_t *ptr, const char* version);
    extern LIBRARY_EXPORT bool abnf_set_protocol(abnf_t *ptr, abnf_protcol_usage_t usage);  

#ifdef __cplusplus
}
#endif

#endif
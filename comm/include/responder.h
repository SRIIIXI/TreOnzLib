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

#ifndef	RESPONDER_C
#define	RESPONDER_C

#include "defines.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct responder_t responder_t;
typedef int socket_t;

extern LIBRARY_EXPORT responder_t* responder_allocate();
extern LIBRARY_EXPORT void responder_free(responder_t* ptr);
extern LIBRARY_EXPORT bool responder_create_socket(responder_t* ptr, const char* servername, int serverport);
extern LIBRARY_EXPORT responder_t* responder_assign_socket(responder_t* ptr, int inSocket);
extern LIBRARY_EXPORT bool responder_connect_socket(responder_t* ptr);
extern LIBRARY_EXPORT bool responder_close_socket(responder_t* ptr);
extern LIBRARY_EXPORT bool responder_send_buffer(responder_t* ptr, const char* data, size_t len);
extern LIBRARY_EXPORT bool responder_send_string(responder_t* ptr, const char* str);

extern LIBRARY_EXPORT bool responder_receive_buffer_by_length(responder_t* ptr, char** iobuffer, size_t len, size_t* out_len, bool alloc_buffer);
extern LIBRARY_EXPORT bool responder_receive_buffer_by_delimeter(responder_t* ptr, char** iobuffer, bool alloc_buffer);

extern LIBRARY_EXPORT bool responder_receive_string(responder_t* ptr, char** iostr, const char* delimeter);

extern LIBRARY_EXPORT size_t responder_get_prefetched_buffer_size(responder_t* ptr);
extern LIBRARY_EXPORT bool responder_is_connected(responder_t* ptr);
extern LIBRARY_EXPORT socket_t responder_get_socket(responder_t* ptr);
extern LIBRARY_EXPORT int responder_get_error_code(responder_t* ptr);
extern LIBRARY_EXPORT void responder_clear_buffer(responder_t* ptr);


#ifdef __cplusplus
}
#endif

#endif


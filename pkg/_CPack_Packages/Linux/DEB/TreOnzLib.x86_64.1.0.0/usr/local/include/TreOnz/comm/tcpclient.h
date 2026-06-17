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

#ifndef TCP_CLIENT_C
#define TCP_CLIENT_C

#include "defines.h"
#include "buffer.h"
#include "stringex.h"

#ifdef __cplusplus
extern "C"
{
#endif

  typedef struct tcp_client_t tcp_client_t;
  typedef int socket_t;

  extern LIBRARY_EXPORT tcp_client_t *tcp_client_allocate();
  extern LIBRARY_EXPORT void tcp_client_free(tcp_client_t *ptr);
  extern LIBRARY_EXPORT bool tcp_client_create_socket(tcp_client_t *ptr, const char *servername, int serverport);
  extern LIBRARY_EXPORT bool tcp_client_connect_socket(tcp_client_t *ptr);
  extern LIBRARY_EXPORT bool tcp_client_switch_to_tls(tcp_client_t *ptr);
  extern LIBRARY_EXPORT bool tcp_client_close_socket(tcp_client_t *ptr);
  extern LIBRARY_EXPORT bool tcp_client_is_connected(tcp_client_t *ptr);

  extern LIBRARY_EXPORT bool tcp_client_send_buffer(tcp_client_t *ptr, const buffer_t *data);
  extern LIBRARY_EXPORT bool tcp_client_send_string(tcp_client_t *ptr, const string_t *str);

  extern LIBRARY_EXPORT buffer_t* tcp_client_receive_buffer_by_length(tcp_client_t *ptr, size_t len);
  extern LIBRARY_EXPORT buffer_t* tcp_client_receive_buffer_by_delimeter(tcp_client_t *ptr, const char *delimeter, size_t delimeterlen);

  extern LIBRARY_EXPORT string_t* tcp_client_receive_string_chunked(tcp_client_t *ptr, const char *delimeter);
  extern LIBRARY_EXPORT string_t* tcp_client_receive_string_precise(tcp_client_t *ptr, const char *delimeter);

  extern LIBRARY_EXPORT socket_t tcp_client_get_socket(tcp_client_t *ptr);
  extern LIBRARY_EXPORT int tcp_client_get_error_code(tcp_client_t *ptr);

#ifdef __cplusplus
}
#endif

#endif

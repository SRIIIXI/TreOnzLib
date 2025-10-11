/*
BSD 2-Clause License

Copyright (c) 2025, Subrato Roy (subratoroy@hotmail.com)
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

#ifndef MQTT_C
#define MQTT_C

#include "defines.h"
#include "buffer.h"
#include "stringex.h"
#include "tcpclient.h"

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct mqtt_client_t mqtt_client_t;

typedef void (*mqtt_message_callback_t)(const string_t* topic, const buffer_t* payload, void* userdata);

/* Client lifecycle */
extern LIBRARY_EXPORT mqtt_client_t* mqtt_client_allocate(const char* client_id);
extern LIBRARY_EXPORT void mqtt_client_free(mqtt_client_t** client);

/* Connection */
extern LIBRARY_EXPORT bool mqtt_client_connect(mqtt_client_t* client, const char* host, int port, bool use_tls, int keepalive_seconds);
extern LIBRARY_EXPORT bool mqtt_client_disconnect(mqtt_client_t* client);
extern LIBRARY_EXPORT bool mqtt_client_is_connected(mqtt_client_t* client);

/* Publish */
extern LIBRARY_EXPORT bool mqtt_client_publish(mqtt_client_t* client, const char* topic, const buffer_t* payload, uint8_t qos, bool retain);

/* Subscribe */
extern LIBRARY_EXPORT bool mqtt_client_subscribe(mqtt_client_t* client, const char* topic, mqtt_message_callback_t callback, void* userdata);
extern LIBRARY_EXPORT bool mqtt_client_unsubscribe(mqtt_client_t* client, const char* topic);

/* Event loop / processing */
extern LIBRARY_EXPORT void mqtt_client_loop(mqtt_client_t* client, int timeout_ms);

#ifdef __cplusplus
}
#endif

#endif

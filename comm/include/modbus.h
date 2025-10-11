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

#ifndef MODBUS_H
#define MODBUS_H

#include "defines.h"
#include "buffer.h"
#include "haltypes.h"
#include "uart.h"
#include "tcpclient.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum modbus_transport_t
{
    MODBUS_TRANSPORT_TCP,
    MODBUS_TRANSPORT_RTU
} modbus_transport_t;

typedef enum modbus_data_type_t {
    MODBUS_DATA_COIL,
    MODBUS_DATA_DISCRETE_INPUT,
    MODBUS_DATA_HOLDING_REGISTER,
    MODBUS_DATA_INPUT_REGISTER
} modbus_data_type_t;


/* Opaque Modbus client type */
typedef struct modbus_client_t modbus_client_t;

/* Allocate / Free */
extern LIBRARY_EXPORT modbus_client_t* modbus_client_allocate(void);
extern LIBRARY_EXPORT void modbus_client_free(modbus_client_t* client);

/* Connect / Disconnect */
extern LIBRARY_EXPORT bool modbus_client_connect_tcp(modbus_client_t* client, const char* hostname, int port);
extern LIBRARY_EXPORT bool modbus_client_connect_rtu(modbus_client_t* client, const char* serial_path, uint32_t baudrate, bool parity_enabled, uart_parity_t parity, uint8_t stopbits, uint8_t data_bits, bool flow_control_enabled, uart_flow_control_t flow_control, uint32_t timeout_ms);
extern LIBRARY_EXPORT bool modbus_client_disconnect(modbus_client_t* client);

/* Send / Receive Modbus requests */
extern LIBRARY_EXPORT bool modbus_client_send_request(modbus_client_t* client, const buffer_t* request);
extern LIBRARY_EXPORT buffer_t* modbus_client_receive_response(modbus_client_t* client, uint32_t timeout_ms);

/* Convenience functions for common operations */
bool modbus_read(modbus_client_t* client, modbus_data_type_t type, uint16_t address, uint16_t quantity, buffer_t** response);
bool modbus_write(modbus_client_t* client, modbus_data_type_t type, uint16_t address, const buffer_t* values);

/* Unit ID */
extern LIBRARY_EXPORT void modbus_set_unit_id(modbus_client_t* client, uint8_t unit_id);

#ifdef __cplusplus
}
#endif

#endif


/*
BSD 2-Clause License

Copyright (c) 2017, Subrato Roy (subratoroy@hotmail.com)
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice,
  this list of conditions and the following disclaimer.

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

#include "modbus.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* CRC16 Modbus helper */
static uint16_t modbus_crc16(const uint8_t* data, size_t len)
{
    uint16_t crc = 0xFFFF;
    for (size_t i = 0; i < len; i++)
    {
        crc ^= data[i];
        for (int j = 0; j < 8; j++)
        {
            if (crc & 0x0001)
                crc = (crc >> 1) ^ 0xA001;
            else
                crc = crc >> 1;
        }
    }
    return crc;
}

/* Internal client structure */
struct modbus_client_t
{
    modbus_transport_t transport;

    tcp_client_t* tcp;
    hal_device_id_t rtu;

    uint8_t unit_id;
    uint16_t transaction_id;
    buffer_t* read_buffer;
    buffer_t* write_buffer;
    bool connected;
};

/* Allocate / Free */
modbus_client_t* modbus_client_allocate(void)
{
    modbus_client_t* client = (modbus_client_t*)malloc(sizeof(modbus_client_t));
    if (!client) return NULL;

    client->transport = MODBUS_TRANSPORT_TCP;
    client->tcp = NULL;
    client->rtu = 0;
    client->unit_id = 1;
    client->transaction_id = 0;
    client->read_buffer = buffer_allocate_default();
    client->write_buffer = buffer_allocate_default();
    client->connected = false;

    return client;
}

void modbus_client_free(modbus_client_t* client)
{
    if (!client) return;

    if (client->transport == MODBUS_TRANSPORT_TCP && client->tcp)
    {
        tcp_client_close_socket(client->tcp);
        tcp_client_free(client->tcp);
    }

    if (client->read_buffer) buffer_free(&client->read_buffer);
    if (client->write_buffer) buffer_free(&client->write_buffer);

    free(client);
}

/* Connect TCP */
bool modbus_client_connect_tcp(modbus_client_t* client, const char* hostname, int port)
{
    if (!client || !hostname) return false;

    client->transport = MODBUS_TRANSPORT_TCP;
    client->tcp = tcp_client_allocate();
    if (!client->tcp) return false;

    if (!tcp_client_create_socket(client->tcp, hostname, port)) return false;
    if (!tcp_client_connect_socket(client->tcp)) return false;

    client->connected = true;
    return true;
}

/* Connect RTU */
bool modbus_client_connect_rtu(modbus_client_t* client, const char* serial_path, uint32_t baudrate, bool parity_enabled, uart_parity_t parity, uint8_t stopbits, uint8_t data_bits, bool flow_control_enabled, uart_flow_control_t flow_control, uint32_t timeout_ms)
{
    if (!client || !serial_path) return false;

    client->transport = MODBUS_TRANSPORT_RTU;

    // Allocate internal UART handle
    hal_device_info_t* devices = NULL;
    size_t device_count = 0;

    if (!uart_enumerate(&devices, &device_count) || device_count == 0)
    {
        return false;
    }

    hal_device_id_t uart_device = HAL_DEVICE_TYPE_UART;
    bool found = false;

    for (size_t i = 0; i < device_count; i++)
    {
        if (strcmp(devices[i].path, serial_path) == 0)
        {
            client->rtu = devices[i].device_id;
            found = true;
            break;
        }
    }

    for (size_t i = 0; i < device_count; i++)
    {
        free(devices[i].metadata);
        // Do not free devices[i] itself, only metadata
    }

    free(devices);

    if (!found)
    {   
        return false;
    }

    // Open serial port
    if (!uart_open(client->rtu))
    {
        return false;
    }

    // Configure UART settings from client struct
    uart_set_baudrate(client->rtu, baudrate);
    uart_set_parity(client->rtu, parity);
    uart_set_stopbits(client->rtu, stopbits);
    uart_set_data_bits(client->rtu, data_bits);
    uart_set_flow_control(client->rtu, flow_control);
    uart_set_timeout(client->rtu, timeout_ms);

    client->connected = true;
    return true;
}


/* Disconnect */
bool modbus_client_disconnect(modbus_client_t* client)
{
    if (!client) return false;

    if (client->transport == MODBUS_TRANSPORT_TCP && client->tcp)
    {
        tcp_client_close_socket(client->tcp);
        tcp_client_free(client->tcp);
        client->tcp = NULL;
    }
    else
    {
        modbus_client_disconnect_rtu(client);
    }

    client->connected = false;
    return true;
}

/* Disconnect RTU */
bool modbus_client_disconnect_rtu(modbus_client_t* client)
{
    if (!client || !client->rtu) return false;

    // Close the UART port
    uart_close(client->rtu);

    // Free the internal UART handle
    uart_free(&client->rtu);

    client->rtu = NULL;
    client->connected = false;
    return true;
}

/* Build TCP frame */
static buffer_t* modbus_build_tcp_frame(modbus_client_t* client, uint8_t function, const uint8_t* data, size_t datalen)
{
    uint16_t length = 1 + datalen;
    uint8_t header[7];
    client->transaction_id++;
    header[0] = client->transaction_id >> 8;
    header[1] = client->transaction_id & 0xFF;
    header[2] = 0;
    header[3] = 0;
    header[4] = length >> 8;
    header[5] = length & 0xFF;
    header[6] = client->unit_id;

    buffer_t* frame = buffer_allocate_length(7 + 1 + datalen);
    buffer_append(frame, header, 7);
    buffer_append(frame, &function, 1);
    if (data && datalen) buffer_append(frame, data, datalen);

    return frame;
}

/* Build RTU frame */
static buffer_t* modbus_build_rtu_frame(modbus_client_t* client, uint8_t function, const uint8_t* data, size_t datalen)
{
    size_t total_len = 1 + 1 + datalen + 2;
    buffer_t* frame = buffer_allocate_length(total_len);

    buffer_append(frame, &client->unit_id, 1);
    buffer_append(frame, &function, 1);
    if (data && datalen) buffer_append(frame, data, datalen);

    uint8_t* ptr = (uint8_t*)buffer_get_data(frame);
    uint16_t crc = modbus_crc16(ptr, 2 + datalen);
    uint8_t crc_bytes[2] = { crc & 0xFF, crc >> 8 };
    buffer_append(frame, crc_bytes, 2);

    return frame;
}

/* Send request */
bool modbus_client_send_request(modbus_client_t* client, const buffer_t* request)
{
    if (!client || !client->connected || !request) return false;

    if (client->transport == MODBUS_TRANSPORT_TCP && client->tcp)
        return tcp_client_send_buffer(client->tcp, request);
    else if (client->transport == MODBUS_TRANSPORT_RTU)
        return uart_write(client->rtu, buffer_get_data(request), buffer_get_size(request));

    return false;
}

/* Receive response */
buffer_t* modbus_client_receive_response(modbus_client_t* client, uint32_t timeout_ms)
{
    if (!client || !client->connected) return NULL;

    buffer_t* response = buffer_allocate_default();

    if (client->transport == MODBUS_TRANSPORT_TCP && client->tcp)
    {
        buffer_t* header = tcp_client_receive_buffer_by_length(client->tcp, 7);
        if (!header) { buffer_free(&response); return NULL; }

        uint16_t len = ((uint8_t*)buffer_get_data(header))[4] << 8 | ((uint8_t*)buffer_get_data(header))[5];
        buffer_free(&header);

        buffer_t* pdu = tcp_client_receive_buffer_by_length(client->tcp, len);
        if (!pdu) { buffer_free(&response); return NULL; }

        buffer_append(response, buffer_get_data(pdu), buffer_get_size(pdu));
        buffer_free(&pdu);
    }
    else if (client->transport == MODBUS_TRANSPORT_RTU)
    {
        size_t read_len = 256;
        uint8_t temp[256];
        if (!uart_read(client->rtu, temp, read_len)) { buffer_free(&response); return NULL; }

        if (read_len < 5) { buffer_free(&response); return NULL; }

        uint16_t crc_recv = temp[read_len - 2] | (temp[read_len - 1] << 8);
        uint16_t crc_calc = modbus_crc16(temp, read_len - 2);
        if (crc_recv != crc_calc) { buffer_free(&response); return NULL; }

        buffer_append(response, temp, read_len - 2);
    }

    return response;
}

/* Generic read function */
bool modbus_read(modbus_client_t* client, modbus_data_type_t type, uint16_t address, uint16_t quantity, buffer_t** response)
{
    if (!client || !response || quantity == 0) return false;

    uint8_t function = 0;
    uint8_t data[4];
    data[0] = address >> 8;
    data[1] = address & 0xFF;
    data[2] = quantity >> 8;
    data[3] = quantity & 0xFF;

    switch (type)
    {
        case MODBUS_DATA_COIL: function = 0x01; break;            // Read Coils
        case MODBUS_DATA_DISCRETE_INPUT: function = 0x02; break;  // Read Discrete Inputs
        case MODBUS_DATA_HOLDING_REGISTER: function = 0x03; break; // Read Holding Registers
        case MODBUS_DATA_INPUT_REGISTER: function = 0x04; break;   // Read Input Registers
        default: return false;
    }

    buffer_t* frame = (client->transport == MODBUS_TRANSPORT_TCP) ?
        modbus_build_tcp_frame(client, function, data, 4) :
        modbus_build_rtu_frame(client, function, data, 4);

    if (!frame) return false;
    if (!modbus_client_send_request(client, frame)) { buffer_free(&frame); return false; }
    buffer_free(&frame);

    buffer_t* resp = modbus_client_receive_response(client, 1000);
    if (!resp) return false;

    *response = resp;
    return true;
}

/* Generic write function */
bool modbus_write(modbus_client_t* client, modbus_data_type_t type, uint16_t address, const buffer_t* values)
{
    if (!client || !values) return false;

    uint8_t function = 0;
    uint8_t* data = NULL;
    size_t data_len = 0;

    switch (type)
    {
        case MODBUS_DATA_COIL: function = 0x0F; break; // Write Multiple Coils
        case MODBUS_DATA_HOLDING_REGISTER: function = 0x10; break; // Write Multiple Registers
        default: return false; // Single write or discrete inputs not supported in generic
    }

    if (function == 0x0F) // Coils
    {
        size_t bit_count = buffer_get_size(values);
        size_t byte_count = (bit_count + 7) / 8;
        data_len = 5 + byte_count;
        data = (uint8_t*)malloc(data_len);
        if (!data) return false;

        data[0] = address >> 8;
        data[1] = address & 0xFF;
        data[2] = bit_count >> 8;
        data[3] = bit_count & 0xFF;
        data[4] = byte_count;

        // Pack coil bits LSB first
        memset(&data[5], 0, byte_count);
        const uint8_t* bits = (const uint8_t*)buffer_get_data(values);
        for (size_t i = 0; i < bit_count; i++)
        {
            if (bits[i]) data[5 + (i / 8)] |= (1 << (i % 8));
        }
    }
    else if (function == 0x10) // Registers
    {
        data_len = 5 + buffer_get_size(values);
        data = (uint8_t*)malloc(data_len);
        if (!data) return false;

        size_t qty = buffer_get_size(values) / 2;
        data[0] = address >> 8;
        data[1] = address & 0xFF;
        data[2] = qty >> 8;
        data[3] = qty & 0xFF;
        data[4] = qty * 2; // byte count
        memcpy(&data[5], buffer_get_data(values), buffer_get_size(values));
    }

    buffer_t* frame = (client->transport == MODBUS_TRANSPORT_TCP) ?
        modbus_build_tcp_frame(client, function, data, data_len) :
        modbus_build_rtu_frame(client, function, data, data_len);

    free(data);
    if (!frame) return false;

    bool ok = modbus_client_send_request(client, frame);
    buffer_free(&frame);
    if (!ok) return false;

    buffer_t* resp = modbus_client_receive_response(client, 1000);
    if (!resp) return false;
    buffer_free(&resp);

    return true;
}

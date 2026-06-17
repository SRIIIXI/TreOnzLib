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

#ifndef VERTEX_BLE_H
#define VERTEX_BLE_H

#include <stdint.h>
#include <stddef.h>
#include "buffer.h"
#include "string.h"
#include "hal.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ble_device ble_device_t;

/* BLE transport type */
typedef enum ble_transport_t
{
    BLE_UART,       /* UART/Serial BLE modules */
    BLE_USB_CDC,    /* USB CDC BLE modules */
    BLE_HCI         /* Native HCI transport (Linux BlueZ, raw HCI) */
} ble_transport_t;

/* BLE open options */
typedef struct ble_options_t
{
    ble_transport_t transport;
    const char* path;           /* UART device path, USB CDC path, or HCI adapter (e.g., "hci0") */
    hal_device_id_t hal_dev_id; /* Pointer to pre-enumerated HAL device */
    uint32_t baudrate;          /* UART only */
} ble_options_t;

/* BLE event callback */
typedef void(*ble_event_cb_t)(ble_device_t* dev, const uint8_t* data, size_t len, void* user_ctx);

/* BLE device management */
extern LIBRARY_EXPORT ble_device_t* ble_open(const ble_options_t* options);
extern LIBRARY_EXPORT void ble_close(ble_device_t* dev);

/* HCI level commands */
extern LIBRARY_EXPORT int ble_send_cmd(ble_device_t* dev, const uint8_t* cmd, size_t len);
extern LIBRARY_EXPORT int ble_receive_event(ble_device_t* dev, uint8_t* buf, size_t max_len, uint32_t timeout_ms);

/* BLE scanning and connection */
extern LIBRARY_EXPORT int ble_scan_start(ble_device_t* dev);
extern LIBRARY_EXPORT int ble_scan_stop(ble_device_t* dev);
extern LIBRARY_EXPORT int ble_connect(ble_device_t* dev, const uint8_t* addr);
extern LIBRARY_EXPORT int ble_connect_service(ble_device_t* dev, const uint8_t* addr, const uint8_t* service_uuid, size_t uuid_len);
extern LIBRARY_EXPORT int ble_disconnect(ble_device_t* dev);

/* GATT wrapper (central mode) */
extern LIBRARY_EXPORT int ble_gatt_write(ble_device_t* dev, const uint8_t* handle, const uint8_t* data, size_t len);
extern LIBRARY_EXPORT int ble_gatt_read(ble_device_t* dev, const uint8_t* handle, uint8_t* data, size_t max_len);

/* Peripheral role advertisement options */
typedef struct
{
    const uint8_t* service_uuid;   /* 16 or 128 bit */
    size_t uuid_len;
    const char* name;              /* Device name */
    uint16_t adv_interval_ms;      /* Advertising interval */
} ble_adv_options_t;

/* Start/stop peripheral advertising */
extern LIBRARY_EXPORT int ble_advertise_start(ble_device_t* dev, const ble_adv_options_t* adv_opts);
extern LIBRARY_EXPORT int ble_advertise_stop(ble_device_t* dev);

/* Peripheral mode GATT callbacks */
typedef void(*ble_gatt_write_cb_t)(const uint8_t* handle, const uint8_t* data, size_t len, void* user_ctx);

typedef void(*ble_gatt_read_cb_t)(const uint8_t* handle, uint8_t* data, size_t max_len, void* user_ctx);

/* Register GATT callbacks (peripheral mode) */
extern LIBRARY_EXPORT void ble_set_gatt_callbacks(ble_device_t* dev,  ble_gatt_read_cb_t read_cb,  ble_gatt_write_cb_t write_cb, void* user_ctx);

/* Event callback registration */
extern LIBRARY_EXPORT void ble_set_event_callback(ble_device_t* dev, ble_event_cb_t cb, void* user_ctx);

#ifdef __cplusplus
}
#endif

#endif /* VERTEX_BLE_H */


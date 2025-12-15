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
 
#include "ble.h"
#include "uart.h"
#include "hal.h"
#include "usb.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#ifdef __linux__
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/select.h>
//#include <linux/bluetooth/hci.h>
//#include <linux/bluetooth/hci_lib.h>
#include <sys/socket.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#endif

#ifdef __FreeBSD__
#include <bluetooth.h>
#include <netgraph/bluetooth/include/ng_btsocket.h>
#include <sys/socket.h>
#endif

/* --- Internal HCI constants --- */
#define HCI_EVENT_PACKET 0x04
#define HCI_LE_META_EVENT 0x3E
#define HCI_LE_ADVERTISING_REPORT 0x02

/* --- BLE device opaque structure --- */
struct ble_device
{
    ble_transport_t transport;
    hal_device_id_t hal_dev;               /* UART, USB CDC, or HCI handle */

    ble_event_cb_t event_cb;
    void* event_ctx;

    /* Peripheral callbacks */
    ble_gatt_read_cb_t gatt_read_cb;
    ble_gatt_write_cb_t gatt_write_cb;
    void* gatt_ctx;
};

/*----------------Internalfor HCI---------------------*/

void hci_open(hal_device_id_t devid);
void hci_close(hal_device_id_t devid);
int hci_write(hal_device_id_t devid, const uint8_t* cmd, size_t len);
int hci_read(hal_device_id_t devid, uint8_t* buf, size_t max_len, uint32_t timeout_ms);

/* ---------------- BLE OPEN / CLOSE ---------------- */

ble_device_t* ble_open(const ble_options_t* options)
{
    if (!options || !options->path)
    {
        return NULL;
    }

    ble_device_t* dev = (ble_device_t*)malloc(sizeof(ble_device_t));
    if (!dev)
    {
        return NULL;
    }

    memset(dev, 0, sizeof(ble_device_t));
    dev->transport = options->transport;

    switch (options->transport)
    {
        case BLE_UART:
            uart_open(options->hal_dev_id);
            uart_set_baudrate(options->hal_dev_id, options->baudrate);
            dev->hal_dev = options->hal_dev_id;
            break;

        case BLE_USB_CDC:
            usb_open(options->hal_dev_id);
            dev->hal_dev = options->hal_dev_id;
            break;

        case BLE_HCI:
            hci_open(options->hal_dev_id);
            break;

        default:
            free(dev);
            return NULL;
    }

    if (!dev->hal_dev)
    {
        free(dev);
        return NULL;
    }

    return dev;
}

void ble_close(ble_device_t* dev)
{
    if (!dev)
    {
        return;
    }

    switch (dev->transport)
    {
        case BLE_UART:
            uart_close(dev->hal_dev);
            break;

        case BLE_USB_CDC:
            usb_close(dev->hal_dev);
            break;

        case BLE_HCI:
            hci_close(dev->hal_dev);
            break;

        default:
            break;
    }

    free(dev);
}

/* ---------------- HCI COMMANDS ---------------- */

int ble_send_cmd(ble_device_t* dev, const uint8_t* cmd, size_t len)
{
    if (!dev || !cmd || !len)
        return -1;

    switch (dev->transport)
    {
        case BLE_UART: return uart_write(dev->hal_dev, cmd, len);
        case BLE_USB_CDC: return usb_write(dev->hal_dev, cmd, len, 100);
        case BLE_HCI: return hci_write(dev->hal_dev, cmd, len);
        default: return -1;
    }
}

int ble_receive_event(ble_device_t* dev, uint8_t* buf, size_t max_len, uint32_t timeout_ms)
{
    if (!dev || !buf || !max_len)
        return -1;

    switch (dev->transport)
    {
        case BLE_UART: return uart_read(dev->hal_dev, buf, max_len);
        case BLE_USB_CDC: return usb_read(dev->hal_dev, buf, max_len, timeout_ms);
        case BLE_HCI: return hci_read(dev->hal_dev, buf, max_len, timeout_ms);
        default: return -1;
    }
}

/* ---------------- EVENT CALLBACK ---------------- */

void ble_set_event_callback(ble_device_t* dev, ble_event_cb_t cb, void* user_ctx)
{
    if (!dev) return;
    dev->event_cb = cb;
    dev->event_ctx = user_ctx;
}

/* ---------------- IN-SOURCE HCI PARSER ---------------- */

/* Simplified: Parse LE Advertising Report events and call event_cb */
static void ble_hci_parse_event(ble_device_t* dev, const uint8_t* buf, size_t len)
{
    if (!dev || !buf || len < 2)
        return;

    uint8_t packet_type = buf[0];

    if (packet_type != HCI_EVENT_PACKET)
        return;

    uint8_t evt_code = buf[1];
    if (evt_code == HCI_LE_META_EVENT && len >= 3)
    {
        uint8_t subevt = buf[3];
        if (subevt == HCI_LE_ADVERTISING_REPORT)
        {
            /* buf[4...] contains advertising report */
            if (dev->event_cb)
            {
                dev->event_cb(dev, &buf[4], len - 4, dev->event_ctx);
            }
        }
    }
    else
    {
        if (dev->event_cb)
        {
            dev->event_cb(dev, &buf[2], len - 2, dev->event_ctx);
        }
    }
}

/* Call in main loop to process HCI events */
void ble_poll_events(ble_device_t* dev, uint32_t timeout_ms)
{
    if (!dev)
        return;

    uint8_t buf[256];
    int n = ble_receive_event(dev, buf, sizeof(buf), timeout_ms);
    if (n > 0)
    {
        ble_hci_parse_event(dev, buf, n);
    }
}

/* ---------------- CENTRAL ROLE ---------------- */

int ble_scan_start(ble_device_t* dev)
{
    if (!dev) return -1;

    uint8_t cmd[] = { 0x01, 0x0C, 0x20, 0x01, 0x01 }; /* HCI LE Set Scan Enable: enable=1, filter=1 */
    return ble_send_cmd(dev, cmd, sizeof(cmd));
}

int ble_scan_stop(ble_device_t* dev)
{
    if (!dev) return -1;

    uint8_t cmd[] = { 0x01, 0x0C, 0x20, 0x01, 0x00 }; /* HCI LE Set Scan Enable: enable=0 */
    return ble_send_cmd(dev, cmd, sizeof(cmd));
}

int ble_connect(ble_device_t* dev, const uint8_t* addr)
{
    if (!dev || !addr) return -1;

    /* Simplified placeholder: construct HCI LE Create Connection packet */
    uint8_t cmd[25] = {0};
    cmd[0] = 0x01; /* HCI Command Packet */
    cmd[1] = 0x04; cmd[2] = 0x20; /* LE Create Connection */
    /* Remaining fields would be filled with addr and scan parameters */
    return ble_send_cmd(dev, cmd, sizeof(cmd));
}

int ble_connect_service(ble_device_t* dev, const uint8_t* addr,
                        const uint8_t* service_uuid, size_t uuid_len)
{
    if (!dev || !service_uuid || !uuid_len)
        return -1;

    /* Start scan, filter advertising reports by service UUID */
    ble_scan_start(dev);

    /* Polling loop simplified (would normally be async) */
    for (int i = 0; i < 100; ++i)
    {
        ble_poll_events(dev, 50);
        /* In real code: check each adv report for matching UUID, then connect */
    }

    ble_scan_stop(dev);
    return 0;
}

int ble_disconnect(ble_device_t* dev)
{
    if (!dev) return -1;

    /* HCI Disconnect (placeholder) */
    uint8_t cmd[] = { 0x01, 0x06, 0x04, 0x00, 0x00 }; /* Connection handle=0, reason=0x00 */
    return ble_send_cmd(dev, cmd, sizeof(cmd));
}

/* ---------------- GATT WRAPPERS (CENTRAL) ---------------- */

/* ATT opcodes */
#define ATT_OP_READ_REQ      0x0A
#define ATT_OP_READ_RESP     0x0B
#define ATT_OP_WRITE_REQ     0x12
#define ATT_OP_WRITE_RESP    0x13

/* Send an ATT Write Request to a characteristic handle */
int ble_gatt_write(ble_device_t* dev, const uint8_t* handle, const uint8_t* data, size_t len)
{
    if (!dev || !handle || !data || !len) return -1;

    uint16_t char_handle = handle[0] | (handle[1] << 8);
    uint8_t pkt[512];

    if (len + 3 > sizeof(pkt)) return -1; /* 3 bytes for opcode + handle */

    pkt[0] = ATT_OP_WRITE_REQ;
    pkt[1] = handle[0];
    pkt[2] = handle[1];
    memcpy(&pkt[3], data, len);

    return ble_send_cmd(dev, pkt, len + 3);
}

/* Send an ATT Read Request to a characteristic handle and read response */
int ble_gatt_read(ble_device_t* dev, const uint8_t* handle, uint8_t* data, size_t max_len)
{
    if (!dev || !handle || !data || !max_len) return -1;

    uint16_t char_handle = handle[0] | (handle[1] << 8);
    uint8_t pkt[3];

    pkt[0] = ATT_OP_READ_REQ;
    pkt[1] = handle[0];
    pkt[2] = handle[1];

    int ret = ble_send_cmd(dev, pkt, sizeof(pkt));
    if (ret < 0) return ret;

    /* Receive ATT Response (blocking, simplified) */
    uint8_t resp[512];
    int n = ble_receive_event(dev, resp, sizeof(resp), 1000); /* 1 second timeout */
    if (n < 0) return -1;

    if (n >= 2 && resp[0] == ATT_OP_READ_RESP)
    {
        int copy_len = n - 1;
        if ((size_t)copy_len > max_len) copy_len = max_len;
        memcpy(data, &resp[1], copy_len);
        return copy_len;
    }

    return -1;
}

/* ---------------- PERIPHERAL ROLE ---------------- */

/* HCI LE Advertising data constants */
#define ADV_TYPE_FLAGS           0x01
#define ADV_TYPE_NAME            0x09
#define ADV_TYPE_16BIT_SERVICE   0x03
#define ADV_TYPE_128BIT_SERVICE  0x07

/* Enable advertising: HCI command */
#define HCI_LE_SET_ADV_ENABLE    0x0A
#define HCI_LE_SET_ADV_DATA      0x08

int ble_advertise_start(ble_device_t* dev, const ble_adv_options_t* adv_opts)
{
    if (!dev || !adv_opts || !adv_opts->service_uuid)
        return -1;

    uint8_t adv_data[31];
    size_t offset = 0;

    /* Flags: LE General Discoverable, BR/EDR Not Supported */
    adv_data[offset++] = 2;           /* Length */
    adv_data[offset++] = ADV_TYPE_FLAGS;
    adv_data[offset++] = 0x06;

    /* Device name */
    if (adv_opts->name)
    {
        size_t name_len = strlen(adv_opts->name);
        if (name_len > 31 - offset - 2) name_len = 31 - offset - 2;
        adv_data[offset++] = (uint8_t)(name_len + 1);
        adv_data[offset++] = ADV_TYPE_NAME;
        memcpy(&adv_data[offset], adv_opts->name, name_len);
        offset += name_len;
    }

    /* Service UUID */
    if (adv_opts->uuid_len == 2)
    {
        adv_data[offset++] = 3;  /* Length */
        adv_data[offset++] = ADV_TYPE_16BIT_SERVICE;
        adv_data[offset++] = adv_opts->service_uuid[0];
        adv_data[offset++] = adv_opts->service_uuid[1];
    }
    else if (adv_opts->uuid_len == 16)
    {
        adv_data[offset++] = 17; /* Length */
        adv_data[offset++] = ADV_TYPE_128BIT_SERVICE;
        memcpy(&adv_data[offset], adv_opts->service_uuid, 16);
        offset += 16;
    }

    /* Send HCI LE Set Advertising Data command */
    uint8_t cmd[32 + 2];
    cmd[0] = 0x01; /* HCI Command Packet */
    cmd[1] = 0x08; cmd[2] = 0x20; /* LE Set Advertising Data */
    cmd[3] = 31;   /* Advertising data length */
    memset(&cmd[4], 0, 30);
    memcpy(&cmd[4], adv_data, offset);

    if (ble_send_cmd(dev, cmd, 4 + 31) < 0)
        return -1;

    /* Enable advertising */
    uint8_t enable_cmd[] = { 0x01, 0x0A, 0x20, 0x01, 0x01 }; /* Enable=1 */
    return ble_send_cmd(dev, enable_cmd, sizeof(enable_cmd));
}

int ble_advertise_stop(ble_device_t* dev)
{
    if (!dev) return -1;

    /* HCI LE Set Advertising Enable = 0 */
    uint8_t cmd[] = { 0x01, 0x0A, 0x20, 0x01, 0x00 };
    return ble_send_cmd(dev, cmd, sizeof(cmd));
}

/* ---------------- PERIPHERAL GATT CALLBACKS ---------------- */

void ble_set_gatt_callbacks(ble_device_t* dev,
                            ble_gatt_read_cb_t read_cb,
                            ble_gatt_write_cb_t write_cb,
                            void* user_ctx)
{
    if (!dev) return;
    dev->gatt_read_cb = read_cb;
    dev->gatt_write_cb = write_cb;
    dev->gatt_ctx = user_ctx;
}

void hci_open(hal_device_id_t devid)
{
    int fd = -1;

#ifdef __linux__
    const char* devname = "hci0"; /* assume mapping from devid */
    fd = socket(AF_BLUETOOTH, SOCK_RAW | SOCK_CLOEXEC, BTPROTO_HCI);
    if (fd < 0) return;

    int dev_id = 0;
    // Need to map devid to actual HCI device id
    // For simplicity, assume devid is the device id
    dev_id = (int)devid;
    if (dev_id < 0) { close(fd); return; }

    struct sockaddr_hci addr = { 0 };
    addr.hci_family = AF_BLUETOOTH;
    addr.hci_dev = dev_id;
    if (bind(fd, (struct sockaddr*)&addr, sizeof(addr)) < 0)
    {
        close(fd);
        return;
    }

    /* store fd in devid (or in internal device struct) */
#endif

#ifdef __FreeBSD__
    /* Open HCI socket */
    fd = socket(PF_BLUETOOTH, SOCK_RAW, BLUETOOTH_PROTO_HCI);
    if (fd < 0) return;

    /* bind to device if needed, or set options */
    /* store fd in devid */
#endif
}

void hci_close(hal_device_id_t devid)
{
  
  int fd = (int)devid; 
  /* Need to resolve the fd from hal device id */

  if (fd >= 0)
  {
    close(fd);
  }

  /* Optionally, reset the handle in devid to indicate closed */
  // devid = -1;  // if hal_device_id_t is a pointer/int
}

int hci_write(hal_device_id_t devid, const uint8_t* cmd, size_t len)
{
    if (!cmd || len == 0)
        return -1;

    int fd = (int)devid; /* assume fd stored in devid */
    ssize_t n = write(fd, cmd, len);

    if (n < 0)
    {
        return -1; /* error writing to HCI socket */
    }

    return (int)n; /* number of bytes sent */
  }

int hci_read(hal_device_id_t devid, uint8_t* buf, size_t max_len, uint32_t timeout_ms)
{
    if (!buf || max_len == 0)
      return -1;

    int fd = (int)devid;
    fd_set rfds;
    struct timeval tv;

    FD_ZERO(&rfds);
    FD_SET(fd, &rfds);

    tv.tv_sec  = timeout_ms / 1000;
    tv.tv_usec = (timeout_ms % 1000) * 1000;

    int ret = select(fd + 1, &rfds, NULL, NULL, &tv);
    if (ret < 0)
    {
      return -1; /* select error */
    }
    else if (ret == 0)
    {
      return 0;  /* timeout */
    }

    /* Ready to read */
    ssize_t n = read(fd, buf, max_len);
    if (n < 0)
    {
      if (errno == EINTR) 
        return 0; /* interrupted, treat as no data */
    
      return -1;
    }

    return (int)n; /* number of bytes read */
  }
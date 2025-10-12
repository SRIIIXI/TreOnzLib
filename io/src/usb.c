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
 

#include "usb.h"
#include "haltypes.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <stdint.h>
#include <sys/select.h>
#include <sys/ioctl.h>

#ifdef __linux__
#include <dirent.h>
#include <linux/usbdevice_fs.h>
#include <linux/usb/ch9.h>
#endif

#ifdef __FreeBSD__
#include <dev/usb/usb.h>
#include <dev/usb/usb_ioctl.h>
#endif

#ifndef HAL_MAX_DEVICES
#define HAL_MAX_DEVICES 16
#endif

// -----------------------------------------------------------------------------
// Opaque USB device structure
// -----------------------------------------------------------------------------

typedef struct usb_device_t
{
    int fd;
    bool opened;
    char path[256];
} usb_device_t;

typedef struct hal_usb_device_descriptor
{
    uint8_t  bLength;
    uint8_t  bDescriptorType;
    uint16_t bcdUSB;
    uint8_t  bDeviceClass;      // <-- THIS IS THE DEVICE CLASS
    uint8_t  bDeviceSubClass;
    uint8_t  bDeviceProtocol;
    uint8_t  bMaxPacketSize0;
    uint16_t idVendor;
    uint16_t idProduct;
    uint16_t bcdDevice;
    uint8_t  iManufacturer;
    uint8_t  iProduct;
    uint8_t  iSerialNumber;
    uint8_t  bNumConfigurations;
}hal_usb_device_descriptor;


static usb_device_t usb_devices[HAL_MAX_DEVICES];
static size_t usb_device_count = 0;

// -----------------------------------------------------------------------------
// Initialize USB subsystem
// -----------------------------------------------------------------------------

bool usb_init(void)
{
    memset(usb_devices, 0, sizeof(usb_devices));
    usb_device_count = 0;
    return true;
}

// -----------------------------------------------------------------------------
// Enumerate USB devices
// -----------------------------------------------------------------------------

bool usb_enumerate(hal_device_info_t *list, size_t *count)
{
    if (!list || !count)
        return false;

    size_t n = 0;

#ifdef __linux__
    DIR *dp = opendir("/dev/bus/usb");
    if (!dp)
        return false;

    struct dirent *entry;
    while ((entry = readdir(dp)) != NULL && n < HAL_MAX_DEVICES && n < *count)
    {
        if (entry->d_type != DT_DIR || entry->d_name[0] == '.')
            continue;

        char bus_path[256];
        snprintf(bus_path, sizeof(bus_path), "/dev/bus/usb/%s", entry->d_name);

        DIR *bus_dp = opendir(bus_path);
        if (!bus_dp)
            continue;

        struct dirent *dev_entry;
        while ((dev_entry = readdir(bus_dp)) != NULL && n < HAL_MAX_DEVICES && n < *count)
        {
            if (dev_entry->d_type != DT_CHR || dev_entry->d_name[0] == '.')
                continue;

            char dev_path[256];
            snprintf(dev_path, sizeof(dev_path), "%s/%s", bus_path, dev_entry->d_name);

            int fd = open(dev_path, O_RDWR | O_NONBLOCK);
            if (fd < 0)
                continue;

            struct usb_device_descriptor desc;
            memset(&desc, 0, sizeof(desc));

            // USBDEVFS_GET_DESCRIPTOR requires struct usbdevfs_ctrltransfer
            struct usbdevfs_ctrltransfer ctrl;
            memset(&ctrl, 0, sizeof(ctrl));
            ctrl.bRequestType = 0x80; // Device to host, standard, device
            ctrl.bRequest = 0x06;     // GET_DESCRIPTOR
            ctrl.wValue = (0x01 << 8); // Device descriptor
            ctrl.wIndex = 0;
            ctrl.wLength = sizeof(desc);
            ctrl.data = &desc;
            ctrl.timeout = 1000;

            if (ioctl(fd, USBDEVFS_CONTROL, &ctrl) < 0)
            {
                close(fd);
                continue;
            }

            close(fd);

            // Populate HAL info
            strncpy(usb_devices[n].path, dev_path, sizeof(usb_devices[n].path));
            usb_devices[n].fd = -1;
            usb_devices[n].opened = false;

            snprintf(list[n].path, sizeof(list[n].path), "%s", dev_path);
            snprintf(list[n].name, sizeof(list[n].name), "USB_Device_%zu", n);
            list[n].type = HAL_DEVICE_TYPE_USB;
            list[n].capabilities = HAL_CAP_USB;
            list[n].device_class = desc.bDeviceClass;
            list[n].device_id = n;
            list[n].metadata = NULL;

            n++;
        }

        closedir(bus_dp);
    }

#elif __FreeBSD__
    for (int i = 0; i < HAL_MAX_DEVICES && n < *count; i++)
    {
        char dev_path[32];
        snprintf(dev_path, sizeof(dev_path), "/dev/usb%d", i);
        int fd = open(dev_path, O_RDWR | O_NONBLOCK);
        if (fd < 0)
            continue;

        struct usb_device_descriptor desc;
        memset(&desc, 0, sizeof(desc));

        if (ioctl(fd, USB_GET_DEVICE_DESC, &desc) < 0)
        {
            close(fd);
            continue;
        }

        close(fd);

        strncpy(usb_devices[n].path, dev_path, sizeof(usb_devices[n].path));
        usb_devices[n].fd = -1;
        usb_devices[n].opened = false;

        snprintf(list[n].path, sizeof(list[n].path), "%s", dev_path);
        snprintf(list[n].name, sizeof(list[n].name), "USB_Device_%zu", n);
        list[n].type = HAL_DEVICE_TYPE_USB;
        list[n].capabilities = HAL_CAP_USB;
        list[n].device_class = desc.bDeviceClass;
        list[n].device_id = n;
        list[n].metadata = NULL;

        n++;
    }
#endif

    usb_device_count = n;
    *count = n;
    return n > 0;
}

// -----------------------------------------------------------------------------
// Open USB device internally
// -----------------------------------------------------------------------------

static bool usb_open(hal_device_id_t device_id)
{
    if (device_id >= usb_device_count)
        return false;

    if (usb_devices[device_id].opened)
        return true;

    int fd = open(usb_devices[device_id].path, O_RDWR | O_NONBLOCK);
    if (fd < 0)
        return false;

    usb_devices[device_id].fd = fd;
    usb_devices[device_id].opened = true;
    return true;
}

// -----------------------------------------------------------------------------
// Read from USB device
// -----------------------------------------------------------------------------

bool usb_read(hal_device_id_t device_id, void *buffer, size_t size, int timeout_ms)
{
    if (!buffer || device_id >= usb_device_count)
        return false;

    if (!usb_open(device_id))
        return false;

    int fd = usb_devices[device_id].fd;

    fd_set rfds;
    FD_ZERO(&rfds);
    FD_SET(fd, &rfds);

    struct timeval tv;
    tv.tv_sec = timeout_ms / 1000;
    tv.tv_usec = (timeout_ms % 1000) * 1000;

    int ret = select(fd + 1, &rfds, NULL, NULL, &tv);
    if (ret <= 0)
        return false;

    ssize_t n = read(fd, buffer, size);
    return n == (ssize_t)size;
}

// -----------------------------------------------------------------------------
// Write to USB device
// -----------------------------------------------------------------------------

bool usb_write(hal_device_id_t device_id, const void *buffer, size_t size, int timeout_ms)
{
    if (!buffer || device_id >= usb_device_count)
        return false;

    if (!usb_open(device_id))
        return false;

    int fd = usb_devices[device_id].fd;

    fd_set wfds;
    FD_ZERO(&wfds);
    FD_SET(fd, &wfds);

    struct timeval tv;
    tv.tv_sec = timeout_ms / 1000;
    tv.tv_usec = (timeout_ms % 1000) * 1000;

    int ret = select(fd + 1, NULL, &wfds, NULL, &tv);
    if (ret <= 0)
        return false;

    ssize_t n = write(fd, buffer, size);
    return n == (ssize_t)size;
}

// -----------------------------------------------------------------------------
// Get USB device state
// -----------------------------------------------------------------------------

bool usb_get_state(hal_device_id_t device_id, void *state, size_t size)
{
    if (!state || device_id >= usb_device_count || size != sizeof(uint32_t))
        return false;

    *((uint32_t *)state) = usb_devices[device_id].opened ? 1 : 0;
    return true;
}

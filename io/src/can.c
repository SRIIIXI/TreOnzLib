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

#include "can.h"
#include "haltypes.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#ifdef __linux__
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <linux/can.h>
#include <linux/can/raw.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <dirent.h>
#endif

#ifdef __FreeBSD__
#include <sys/ioctl.h>
#include <net/if.h>
#include <dev/can/can_ioctl.h>
#endif

#ifndef HAL_MAX_DEVICES
#define HAL_MAX_DEVICES 8
#endif

#ifdef __linux__
#define SYSFS_CAN_PATH "/sys/class/net"
#endif

// -----------------------------------------------------------------------------
// Opaque CAN device structure
// -----------------------------------------------------------------------------

typedef struct can_device_t
{
    int fd;
    bool opened;
#ifdef __linux__
    char ifname[IFNAMSIZ];
#endif
} can_device_t;

static can_device_t can_devices[HAL_MAX_DEVICES];
static size_t can_device_count = 0;

// -----------------------------------------------------------------------------
// Initialize CAN subsystem
// -----------------------------------------------------------------------------

bool can_init(void)
{
    memset(can_devices, 0, sizeof(can_devices));
    can_device_count = 0;

#ifdef __linux__
    DIR *dp = opendir(SYSFS_CAN_PATH);
    if (!dp)
    {
        perror("opendir can devices");
        return false;
    }

    struct dirent *entry;
    while ((entry = readdir(dp)) != NULL && can_device_count < HAL_MAX_DEVICES)
    {
        if (strncmp(entry->d_name, "can", 3) != 0)
            continue;

        int s = socket(PF_CAN, SOCK_RAW, CAN_RAW);
        if (s < 0)
            continue;

        struct ifreq ifr;
        strncpy(ifr.ifr_name, entry->d_name, IFNAMSIZ);
        if (ioctl(s, SIOCGIFINDEX, &ifr) < 0)
        {
            close(s);
            continue;
        }

        struct sockaddr_can addr;
        memset(&addr, 0, sizeof(addr));
        addr.can_family = AF_CAN;
        addr.can_ifindex = ifr.ifr_ifindex;

        if (bind(s, (struct sockaddr *)&addr, sizeof(addr)) < 0)
        {
            close(s);
            continue;
        }

        can_devices[can_device_count].fd = s;
        strncpy(can_devices[can_device_count].ifname, entry->d_name, IFNAMSIZ);
        can_devices[can_device_count].opened = true;
        can_device_count++;
    }

    closedir(dp);
#elif __FreeBSD__
    for (int i = 0; i < HAL_MAX_DEVICES; i++)
    {
        char path[32];
        snprintf(path, sizeof(path), "/dev/can%d", i);
        int fd = open(path, O_RDWR);
        if (fd >= 0)
        {
            can_devices[can_device_count].fd = fd;
            can_devices[can_device_count].opened = true;
            can_device_count++;
        }
    }
#endif

    return can_device_count > 0;
}

// -----------------------------------------------------------------------------
// Enumerate CAN devices
// -----------------------------------------------------------------------------

bool can_enumerate(hal_device_info_t *list, size_t *count)
{
    if (!list || !count)
        return false;

    size_t n = (*count < can_device_count) ? *count : can_device_count;
    for (size_t i = 0; i < n; i++)
    {
#ifdef __linux__
        snprintf(list[i].path, sizeof(list[i].path), "%s", can_devices[i].ifname);
#else
        snprintf(list[i].path, sizeof(list[i].path), "/dev/can%zu", i);
#endif
        snprintf(list[i].name, sizeof(list[i].name), "CAN_Device_%zu", i);
        list[i].type = HAL_DEVICE_TYPE_CAN;
        list[i].capabilities = HAL_CAP_CAN;
        list[i].metadata = NULL;
    }
    *count = n;
    return true;
}

// -----------------------------------------------------------------------------
// Send a CAN frame (HAL write semantic)
// -----------------------------------------------------------------------------

bool can_write(hal_device_id_t device_id, void *frame, size_t size)
{
    if (device_id >= can_device_count || !frame || size != sizeof(struct can_frame))
        return false;

#ifdef __linux__
    ssize_t n = write(can_devices[device_id].fd, frame, sizeof(struct can_frame));
    return n == sizeof(struct can_frame);
#elif __FreeBSD__
    struct can_msg *msg = (struct can_msg *)frame;
    return ioctl(can_devices[device_id].fd, CAN_WRITE, msg) == 0;
#endif
}

// -----------------------------------------------------------------------------
// Receive a CAN frame (HAL read semantic)
// -----------------------------------------------------------------------------

bool can_read(hal_device_id_t device_id, void *frame, size_t size, int timeout_ms)
{
    if (device_id >= can_device_count || !frame || size != sizeof(struct can_frame))
        return false;

#ifdef __linux__
    fd_set rfds;
    struct timeval tv;
    FD_ZERO(&rfds);
    FD_SET(can_devices[device_id].fd, &rfds);
    tv.tv_sec = timeout_ms / 1000;
    tv.tv_usec = (timeout_ms % 1000) * 1000;

    int ret = select(can_devices[device_id].fd + 1, &rfds, NULL, NULL, &tv);
    if (ret <= 0)
        return false;

    ssize_t n = read(can_devices[device_id].fd, frame, sizeof(struct can_frame));
    return n == sizeof(struct can_frame);
#elif __FreeBSD__
    struct can_msg *msg = (struct can_msg *)frame;
    // Timeout handling can be implemented later via ioctl or poll
    return ioctl(can_devices[device_id].fd, CAN_READ, msg) == 0;
#endif
}

// -----------------------------------------------------------------------------
// Set / Get bitrate (via opaque pointer)
// -----------------------------------------------------------------------------

bool can_set_bitrate(hal_device_id_t device_id, void *config, size_t size)
{
    if (device_id >= can_device_count || !config || size != sizeof(uint32_t))
        return false;

    uint32_t bitrate = *((uint32_t *)config);

#ifdef __linux__
    // Use system call: ip link set canX type can bitrate <bitrate>
    // For simplicity, skipped here (user can configure via shell)
    (void)bitrate;
    return true;
#elif __FreeBSD__
    struct can_bitrate bc;
    bc.bitrate = bitrate;
    return ioctl(can_devices[device_id].fd, CAN_SET_BITRATE, &bc) == 0;
#endif
}

bool can_get_bitrate(hal_device_id_t device_id, void *config, size_t size)
{
    if (device_id >= can_device_count || !config || size != sizeof(uint32_t))
        return false;

#ifdef __linux__
    *((uint32_t *)config) = 500000; // default 500kbps, as placeholder
    return true;
#elif __FreeBSD__
    struct can_bitrate bc;
    if (ioctl(can_devices[device_id].fd, CAN_GET_BITRATE, &bc) != 0)
        return false;
    *((uint32_t *)config) = bc.bitrate;
    return true;
#endif
}

// -----------------------------------------------------------------------------
// Get CAN state (active, error, bus off)
// -----------------------------------------------------------------------------

bool can_get_state(hal_device_id_t device_id, void *config, size_t size)
{
    if (device_id >= can_device_count || !config || size != sizeof(uint32_t))
        return false;

#ifdef __linux__
    // Placeholder: 0 = active
    *((uint32_t *)config) = 0;
    return true;
#elif __FreeBSD__
    struct can_state cs;
    if (ioctl(can_devices[device_id].fd, CAN_GET_STATE, &cs) != 0)
        return false;
    *((uint32_t *)config) = cs.state;
    return true;
#endif
}

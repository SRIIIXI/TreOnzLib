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

#include "dac.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <stdint.h>

#ifdef __linux__
#include <dirent.h>
#endif

#ifdef __FreeBSD__
#include <sys/ioctl.h>
#endif

#ifndef HAL_MAX_DEVICES
#define HAL_MAX_DEVICES 8
#endif

#ifdef __linux__
#define SYSFS_DAC_PATH "/sys/bus/iio/devices"
#endif

// -----------------------------------------------------------------------------
// Opaque DAC device structure
// -----------------------------------------------------------------------------

typedef struct dac_device_t
{
#ifdef __linux__
    int fd;
    char path[256];
#elif __FreeBSD__
    int fd;
#endif
    bool opened;
} dac_device_t;

static dac_device_t dac_devices[HAL_MAX_DEVICES];
static size_t dac_device_count = 0;

// -----------------------------------------------------------------------------
// Helpers for Linux
// -----------------------------------------------------------------------------

#ifdef __linux__
static int dac_fd_open(const char *path)
{
    int fd = open(path, O_WRONLY);
    if (fd < 0)
    {
        perror(path);
    }
    return fd;
}

static bool dac_fd_write(int fd, uint32_t value)
{
    if (fd < 0)
    {
        return false;
    }

    char buf[32];
    snprintf(buf, sizeof(buf), "%u", value);
    ssize_t n = write(fd, buf, strlen(buf));
    return n == (ssize_t)strlen(buf);
}
#endif

// -----------------------------------------------------------------------------
// Initialize DAC
// -----------------------------------------------------------------------------

bool dac_init(void)
{
    memset(dac_devices, 0, sizeof(dac_devices));
    dac_device_count = 0;

#ifdef __linux__
    DIR *dp = opendir(SYSFS_DAC_PATH);
    if (!dp)
    {
        perror("opendir iio devices");
        return false;
    }

    struct dirent *entry;
    while ((entry = readdir(dp)) != NULL && dac_device_count < HAL_MAX_DEVICES)
    {
        if (strncmp(entry->d_name, "iio:device", 10) != 0)
        {
            continue;
        }

        // Open channel 0 for simplicity
        char path[256];
        snprintf(path, sizeof(path), "%s/%s/out_voltage0_raw", SYSFS_DAC_PATH, entry->d_name);

        int fd = dac_fd_open(path);
        if (fd >= 0)
        {
            dac_devices[dac_device_count].fd = fd;
            strncpy(dac_devices[dac_device_count].path, path, sizeof(dac_devices[dac_device_count].path));
            dac_devices[dac_device_count].opened = true;
            dac_device_count++;
        }
    }

    closedir(dp);
#elif __FreeBSD__
    for (int i = 0; i < HAL_MAX_DEVICES; i++)
    {
        char path[32];
        snprintf(path, sizeof(path), "/dev/dac%d", i);
        int fd = open(path, O_RDWR);
        if (fd >= 0)
        {
            dac_devices[dac_device_count].fd = fd;
            dac_devices[dac_device_count].opened = true;
            dac_device_count++;
        }
    }
#endif

    return dac_device_count > 0;
}

// -----------------------------------------------------------------------------
// Enumerate DAC devices
// -----------------------------------------------------------------------------

bool dac_enumerate(hal_device_info_t *list, size_t *count)
{
    if (!list || !count)
    {
        return false;
    }

    size_t n = (*count < dac_device_count) ? *count : dac_device_count;
    for (size_t i = 0; i < n; i++)
    {
        snprintf(list[i].path, sizeof(list[i].path), "%s", dac_devices[i].opened ? "/dev/dac" : "");
        snprintf(list[i].name, sizeof(list[i].name), "DAC_Device_%zu", i);
        list[i].type = HAL_DEVICE_TYPE_DAC;
        list[i].capabilities = HAL_CAP_DAC;
        list[i].metadata = NULL;
    }
    *count = n;
    return true;
}

// -----------------------------------------------------------------------------
// Write DAC value
// -----------------------------------------------------------------------------

bool dac_write(hal_device_id_t device_id, void *value, size_t size)
{
    (void)size;
    if (device_id >= dac_device_count || !value)
    {
        return false;
    }

    uint32_t val = *((uint32_t *)value);

#ifdef __linux__
    return dac_fd_write(dac_devices[device_id].fd, val);
#elif __FreeBSD__
    // struct dac_ioc_write wr;
    // wr.channel = 0;
    // wr.value = val;
    // return ioctl(dac_devices[device_id].fd, DAC_WRITE, &wr) == 0;
#endif
}

// -----------------------------------------------------------------------------
// Get DAC resolution (bits)
// -----------------------------------------------------------------------------

bool dac_get_resolution(hal_device_id_t device_id, void *value, size_t size)
{
    (void)size;
    if (device_id >= dac_device_count || !value)
    {
        return false;
    }

#ifdef __linux__
    // Assume 12-bit typical, can be extended to read sysfs if available
    *((uint32_t *)value) = 12;
    return true;
#elif __FreeBSD__
    // struct dac_ioc_cfg cfg;
    // if (ioctl(dac_devices[device_id].fd, DAC_GETRESOLUTION, &cfg) != 0)
    // {
    //     return false;
    // }
    // *((uint32_t *)value) = cfg.resolution;
    return true;
#endif
}

// -----------------------------------------------------------------------------
// Get DAC reference voltage (millivolts)
// -----------------------------------------------------------------------------

bool dac_get_reference_voltage(hal_device_id_t device_id, void *value, size_t size)
{
    (void)size;
    if (device_id >= dac_device_count || !value)
    {
        return false;
    }

#ifdef __linux__
    // Assume 3300 mV typical, extend to read sysfs if available
    *((uint32_t *)value) = 3300;
    return true;
#elif __FreeBSD__
    // struct dac_ioc_cfg cfg;
    // if (ioctl(dac_devices[device_id].fd, DAC_GETVREF, &cfg) != 0)
    // {
    //     return false;
    // }
    // *((uint32_t *)value) = cfg.vref_mv;
    return true;
#endif
}


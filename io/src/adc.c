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

#include "adc.h"
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
#define SYSFS_ADC_PATH "/sys/bus/iio/devices"
#endif

// -----------------------------------------------------------------------------
// Opaque ADC device structure
// -----------------------------------------------------------------------------

typedef struct adc_device_t
{
#ifdef __linux__
    int fd;
    char path[256];
#elif __FreeBSD__
    int fd;
#endif
    bool opened;
} adc_device_t;

static adc_device_t adc_devices[HAL_MAX_DEVICES];
static size_t adc_device_count = 0;

// -----------------------------------------------------------------------------
// Helpers for Linux
// -----------------------------------------------------------------------------

#ifdef __linux__
static int adc_fd_open(const char *path)
{
    int fd = open(path, O_RDONLY);
    if (fd < 0)
    {
        perror(path);
    }
    return fd;
}

static bool adc_fd_read(int fd, char *buf, size_t size)
{
    if (fd < 0)
    {
        return false;
    }
    lseek(fd, 0, SEEK_SET);
    ssize_t n = read(fd, buf, size - 1);
    if (n <= 0)
    {
        return false;
    }
    buf[n] = '\0';
    return true;
}
#endif

// -----------------------------------------------------------------------------
// Initialize ADC
// -----------------------------------------------------------------------------

bool adc_init(void)
{
    memset(adc_devices, 0, sizeof(adc_devices));
    adc_device_count = 0;

#ifdef __linux__
    DIR *dp = opendir(SYSFS_ADC_PATH);
    if (!dp)
    {
        perror("opendir iio devices");
        return false;
    }

    struct dirent *entry;
    while ((entry = readdir(dp)) != NULL && adc_device_count < HAL_MAX_DEVICES)
    {
        if (strncmp(entry->d_name, "iio:device", 10) != 0)
        {
            continue;
        }

        // Open channel 0 for simplicity
        char path[256];
        snprintf(path, sizeof(path), "%s/%s/in_voltage0_raw", SYSFS_ADC_PATH, entry->d_name);

        int fd = adc_fd_open(path);
        if (fd >= 0)
        {
            adc_devices[adc_device_count].fd = fd;
            strncpy(adc_devices[adc_device_count].path, path, sizeof(adc_devices[adc_device_count].path));
            adc_devices[adc_device_count].opened = true;
            adc_device_count++;
        }
    }

    closedir(dp);
#elif __FreeBSD__
    for (int i = 0; i < HAL_MAX_DEVICES; i++)
    {
        char path[32];
        snprintf(path, sizeof(path), "/dev/adc%d", i);
        int fd = open(path, O_RDWR);
        if (fd >= 0)
        {
            adc_devices[adc_device_count].fd = fd;
            adc_devices[adc_device_count].opened = true;
            adc_device_count++;
        }
    }
#endif

    return adc_device_count > 0;
}

// -----------------------------------------------------------------------------
// Enumerate ADC devices
// -----------------------------------------------------------------------------

bool adc_enumerate(hal_device_info_t *list, size_t *count)
{
    if (!list || !count)
    {
        return false;
    }

    size_t n = (*count < adc_device_count) ? *count : adc_device_count;
    for (size_t i = 0; i < n; i++)
    {
        snprintf(list[i].path, sizeof(list[i].path), "%s", adc_devices[i].opened ? "/dev/adc" : "");
        snprintf(list[i].name, sizeof(list[i].name), "ADC_Device_%zu", i);
        list[i].type = HAL_DEVICE_TYPE_ADC;
        list[i].capabilities = HAL_CAP_ADC;
        list[i].metadata = NULL;
    }
    *count = n;
    return true;
}

// -----------------------------------------------------------------------------
// Read ADC value
// -----------------------------------------------------------------------------

bool adc_read(hal_device_id_t device_id, void *value, size_t size)
{
    (void)size;
    if (device_id >= adc_device_count || !value)
    {
        return false;
    }

#ifdef __linux__
    char buf[32];
    if (!adc_fd_read(adc_devices[device_id].fd, buf, sizeof(buf)))
    {
        return false;
    }
    *((uint32_t *)value) = (uint32_t)atoi(buf);
    return true;
#elif __FreeBSD__
    // struct adc_ioc_read rd;
    // rd.channel = 0;
    // rd.value = 0;
    // if (ioctl(adc_devices[device_id].fd, ADC_READ, &rd) != 0)
    // {
    //     return false;
    // }
    // *((uint32_t *)value) = rd.value;
    return true;
#endif
}

// -----------------------------------------------------------------------------
// Get ADC resolution (bits)
// -----------------------------------------------------------------------------

bool adc_get_resolution(hal_device_id_t device_id, void *value, size_t size)
{
    (void)size;
    if (device_id >= adc_device_count || !value)
    {
        return false;
    }

#ifdef __linux__
    // Assume 12-bit typical, or read from sysfs if available
    *((uint32_t *)value) = 12;
    return true;
#elif __FreeBSD__
    // struct adc_ioc_cfg cfg;
    // if (ioctl(adc_devices[device_id].fd, ADC_GETRESOLUTION, &cfg) != 0)
    // {
    //     return false;
    // }
    // *((uint32_t *)value) = cfg.resolution;
    return true;
#endif
}

// -----------------------------------------------------------------------------
// Get ADC reference voltage (millivolts)
// -----------------------------------------------------------------------------

bool adc_get_reference_voltage(hal_device_id_t device_id, void *value, size_t size)
{
    (void)size;
    if (device_id >= adc_device_count || !value)
    {
        return false;
    }

#ifdef __linux__
    // Assume 3300 mV typical, or read from sysfs if available
    *((uint32_t *)value) = 3300;
    return true;
#elif __FreeBSD__
    // struct adc_ioc_cfg cfg;
    // if (ioctl(adc_devices[device_id].fd, ADC_GETVREF, &cfg) != 0)
    // {
    //     return false;
    // }
    // *((uint32_t *)value) = cfg.vref_mv;
    return true;
#endif
}

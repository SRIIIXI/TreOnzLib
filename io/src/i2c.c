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

#include "i2c.h"
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>

#ifdef __linux__
#include <linux/i2c-dev.h>
#elif __FreeBSD__
#include <iicbus/iic.h>
#include <sys/ioccom.h>
#endif

#define MAX_I2C_DEVICES 16

typedef struct i2c_t {
    int fd;
    char path[256];
    uint8_t address;
    uint32_t speed_hz;
} i2c_t;

static i2c_t* i2c_devices[MAX_I2C_DEVICES] = {0};
static hal_device_info_t i2c_info[MAX_I2C_DEVICES] = {0};
static size_t i2c_count = 0;

/* Initialize I2C subsystem */
bool i2c_init(void)
{
    for (size_t i = 0; i < MAX_I2C_DEVICES; i++) {
        i2c_devices[i] = NULL;
        memset(&i2c_info[i], 0, sizeof(hal_device_info_t));
    }
    i2c_count = 0;
    return true;
}

/* Enumerate I2C devices */
bool i2c_enumerate(hal_device_info_t *list, size_t *count)
{
    if (!list || !count) return false;
    i2c_init();

#ifdef __linux__
    for (int bus = 0; bus < 8 && i2c_count < MAX_I2C_DEVICES; bus++) {
        char path[64];
        snprintf(path, sizeof(path), "/dev/i2c-%d", bus);
        int fd = open(path, O_RDWR);
        if (fd >= 0) {
            close(fd);
            hal_device_info_t* info = &i2c_info[i2c_count];
            info->device_id = (hal_device_id_t)i2c_count;
            info->type = HAL_DEVICE_TYPE_I2C;
            strncpy(info->path, path, sizeof(info->path)-1);
            snprintf(info->name, sizeof(info->name), "I2C_%d", bus);
            info->capabilities = HAL_CAP_I2C;
            i2c_devices[i2c_count] = NULL;
            i2c_count++;
        }
    }
#elif __FreeBSD__
    for (int bus = 0; bus < 8 && i2c_count < MAX_I2C_DEVICES; bus++) {
        char path[64];
        snprintf(path, sizeof(path), "/dev/iic%d", bus);
        int fd = open(path, O_RDWR);
        if (fd >= 0) {
            close(fd);
            hal_device_info_t* info = &i2c_info[i2c_count];
            info->device_id = (hal_device_id_t)i2c_count;
            info->type = HAL_DEVICE_TYPE_I2C;
            strncpy(info->path, path, sizeof(info->path)-1);
            snprintf(info->name, sizeof(info->name), "I2C_%d", bus);
            info->capabilities = HAL_CAP_I2C;
            i2c_devices[i2c_count] = NULL;
            i2c_count++;
        }
    }
#endif

    size_t to_copy = (*count < i2c_count) ? *count : i2c_count;
    memcpy(list, i2c_info, to_copy * sizeof(hal_device_info_t));
    *count = i2c_count;
    return true;
}

/* Allocate internal i2c_t */
static i2c_t* i2c_allocate(void)
{
    i2c_t* dev = (i2c_t*)malloc(sizeof(i2c_t));
    if (dev) {
        dev->fd = -1;
        dev->address = 0;
        dev->speed_hz = 100000; // default 100 kHz
        dev->path[0] = '\0';
    }
    return dev;
}

/* Free internal i2c_t */
static void i2c_free(i2c_t** dev)
{
    if (!dev || !*dev) return;
    if ((*dev)->fd >= 0) close((*dev)->fd);
    free(*dev);
    *dev = NULL;
}

/* Open I2C device by hal_device_id_t */
bool i2c_open(hal_device_id_t device_id)
{
    if (device_id >= i2c_count) return false;
    if (i2c_devices[device_id]) return true; // already open

    hal_device_info_t* info = &i2c_info[device_id];
    i2c_t* dev = i2c_allocate();
    if (!dev) return false;

    strncpy(dev->path, info->path, sizeof(dev->path)-1);
    dev->fd = open(dev->path, O_RDWR);
    if (dev->fd < 0) {
        i2c_free(&dev);
        return false;
    }

    i2c_devices[device_id] = dev;
    return true;
}

/* Close I2C device */
bool i2c_close(hal_device_id_t device_id)
{
    if (device_id >= i2c_count) return false;
    i2c_t* dev = i2c_devices[device_id];
    if (!dev) return false;
    i2c_free(&dev);
    i2c_devices[device_id] = NULL;
    return true;
}

/* Set slave address */
bool i2c_set_address(hal_device_id_t device_id, uint8_t address)
{
    if (device_id >= i2c_count) return false;
    i2c_t* dev = i2c_devices[device_id];
    if (!dev || dev->fd < 0) return false;
    dev->address = address;

#ifdef __linux__
    if (ioctl(dev->fd, I2C_SLAVE, address) < 0) return false;
#elif __FreeBSD__
    struct iiccmd req;
    req.slave = address;
    req.buf = NULL;
    if (ioctl(dev->fd, I2CSADDR, &req) < 0) return false;
#endif
    return true;
}

/* Set bus speed (stored internally, may not affect device) */
bool i2c_set_speed(hal_device_id_t device_id, uint32_t speed_hz)
{
    if (device_id >= i2c_count) return false;
    i2c_t* dev = i2c_devices[device_id];
    if (!dev) return false;
    dev->speed_hz = speed_hz;
    return true;
}

/* Write to I2C device */
bool i2c_write(hal_device_id_t device_id, const void* data, size_t size)
{
    if (device_id >= i2c_count) return false;
    i2c_t* dev = i2c_devices[device_id];
    if (!dev || dev->fd < 0 || !data || size == 0) return false;
    ssize_t written = write(dev->fd, data, size);
    return written == (ssize_t)size;
}

/* Read from I2C device */
bool i2c_read(hal_device_id_t device_id, void* buffer, size_t size)
{
    if (device_id >= i2c_count) return false;
    i2c_t* dev = i2c_devices[device_id];
    if (!dev || dev->fd < 0 || !buffer || size == 0) return false;
    ssize_t rd = read(dev->fd, buffer, size);
    return rd == (ssize_t)size;
}

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

#include "spi.h"
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>

#ifdef __linux__
#include <linux/spi/spidev.h>
#elif __FreeBSD__
#include <sys/spiio.h>
#endif

#define MAX_SPI_DEVICES 16

typedef struct spi_t {
    int fd;
    char path[256];
    uint8_t mode;
    uint32_t speed_hz;
    uint8_t bits_per_word;
} spi_t;

static spi_t* spi_devices[MAX_SPI_DEVICES] = {0};
static hal_device_info_t spi_info[MAX_SPI_DEVICES] = {0};
static size_t spi_count = 0;

/* Initialize SPI subsystem */
bool spi_init(void)
{
    for (size_t i = 0; i < MAX_SPI_DEVICES; i++) {
        spi_devices[i] = NULL;
        memset(&spi_info[i], 0, sizeof(hal_device_info_t));
    }
    spi_count = 0;
    return true;
}

/* Enumerate SPI devices */
bool spi_enumerate(hal_device_info_t *list, size_t *count)
{
    if (!list || !count) return false;
    spi_init();

#ifdef __linux__
    for (int bus = 0; bus < 4 && spi_count < MAX_SPI_DEVICES; bus++) {
        for (int cs = 0; cs < 4 && spi_count < MAX_SPI_DEVICES; cs++) {
            char path[64];
            snprintf(path, sizeof(path), "/dev/spidev%d.%d", bus, cs);
            int fd = open(path, O_RDWR);
            if (fd >= 0) {
                close(fd);
                hal_device_info_t* info = &spi_info[spi_count];
                info->device_id = (hal_device_id_t)spi_count;
                info->type = HAL_DEVICE_TYPE_SPI;
                strncpy(info->path, path, sizeof(info->path)-1);
                snprintf(info->name, sizeof(info->name), "SPI_%d_%d", bus, cs);
                info->capabilities = HAL_CAP_SPI;
                spi_devices[spi_count] = NULL;
                spi_count++;
            }
        }
    }
#elif __FreeBSD__
    for (int bus = 0; bus < 4 && spi_count < MAX_SPI_DEVICES; bus++) {
        char path[64];
        snprintf(path, sizeof(path), "/dev/spibus%d", bus);
        int fd = open(path, O_RDWR);
        if (fd >= 0) {
            close(fd);
            hal_device_info_t* info = &spi_info[spi_count];
            info->device_id = (hal_device_id_t)spi_count;
            info->type = HAL_DEVICE_TYPE_SPI;
            strncpy(info->path, path, sizeof(info->path)-1);
            snprintf(info->name, sizeof(info->name), "SPI_%d", bus);
            info->capabilities = HAL_CAP_SPI;
            spi_devices[spi_count] = NULL;
            spi_count++;
        }
    }
#endif

    size_t to_copy = (*count < spi_count) ? *count : spi_count;
    memcpy(list, spi_info, to_copy * sizeof(hal_device_info_t));
    *count = spi_count;
    return true;
}

/* Allocate internal SPI device */
static spi_t* spi_allocate(void)
{
    spi_t* dev = (spi_t*)malloc(sizeof(spi_t));
    if (dev) {
        dev->fd = -1;
        dev->mode = 0;
        dev->speed_hz = 500000; // default 500 kHz
        dev->bits_per_word = 8;
        dev->path[0] = '\0';
    }
    return dev;
}

/* Free internal SPI device */
static void spi_free(spi_t** dev)
{
    if (!dev || !*dev) return;
    if ((*dev)->fd >= 0) close((*dev)->fd);
    free(*dev);
    *dev = NULL;
}

/* Open SPI device */
bool spi_open(hal_device_id_t device_id)
{
    if (device_id >= spi_count) return false;
    if (spi_devices[device_id]) return true; // already open

    hal_device_info_t* info = &spi_info[device_id];
    spi_t* dev = spi_allocate();
    if (!dev) return false;

    strncpy(dev->path, info->path, sizeof(dev->path)-1);
    dev->fd = open(dev->path, O_RDWR);
    if (dev->fd < 0) {
        spi_free(&dev);
        return false;
    }

    spi_devices[device_id] = dev;
    return true;
}

/* Close SPI device */
bool spi_close(hal_device_id_t device_id)
{
    if (device_id >= spi_count) return false;
    spi_t* dev = spi_devices[device_id];
    if (!dev) return false;
    spi_free(&dev);
    spi_devices[device_id] = NULL;
    return true;
}

/* Write to SPI */
bool spi_write(hal_device_id_t device_id, const void* data, size_t size)
{
    if (device_id >= spi_count) return false;
    spi_t* dev = spi_devices[device_id];
    if (!dev || dev->fd < 0 || !data || size == 0) return false;

#ifdef __linux__
    struct spi_ioc_transfer tr = {0};
    tr.tx_buf = (unsigned long)data;
    tr.rx_buf = 0;
    tr.len = size;
    tr.speed_hz = dev->speed_hz;
    tr.bits_per_word = dev->bits_per_word;
    tr.delay_usecs = 0;

    if (ioctl(dev->fd, SPI_IOC_MESSAGE(1), &tr) < 1) return false;
#elif __FreeBSD__
    ssize_t wr = write(dev->fd, data, size);
    if (wr != (ssize_t)size) return false;
#endif
    return true;
}

/* Read from SPI */
bool spi_read(hal_device_id_t device_id, void* buffer, size_t size)
{
    if (device_id >= spi_count) return false;
    spi_t* dev = spi_devices[device_id];
    if (!dev || dev->fd < 0 || !buffer || size == 0) return false;

#ifdef __linux__
    struct spi_ioc_transfer tr = {0};
    tr.tx_buf = 0;
    tr.rx_buf = (unsigned long)buffer;
    tr.len = size;
    tr.speed_hz = dev->speed_hz;
    tr.bits_per_word = dev->bits_per_word;
    tr.delay_usecs = 0;

    if (ioctl(dev->fd, SPI_IOC_MESSAGE(1), &tr) < 1) return false;
#elif __FreeBSD__
    ssize_t rd = read(dev->fd, buffer, size);
    if (rd != (ssize_t)size) return false;
#endif
    return true;
}

/* Full-duplex SPI transfer: write tx_buf and read into rx_buf */
bool spi_transfer(hal_device_id_t device_id, const void* tx_buf, void* rx_buf, size_t size)
{
    if (device_id >= spi_count) return false;
    spi_t* dev = spi_devices[device_id];
    if (!dev || dev->fd < 0 || !tx_buf || !rx_buf || size == 0) return false;

#ifdef __linux__
    struct spi_ioc_transfer tr = {0};
    tr.tx_buf = (unsigned long)tx_buf;
    tr.rx_buf = (unsigned long)rx_buf;
    tr.len = size;
    tr.speed_hz = dev->speed_hz;
    tr.bits_per_word = dev->bits_per_word;
    tr.delay_usecs = 0;

    if (ioctl(dev->fd, SPI_IOC_MESSAGE(1), &tr) < 1) return false;
#elif __FreeBSD__
    ssize_t wr = write(dev->fd, tx_buf, size);
    if (wr != (ssize_t)size) return false;
    ssize_t rd = read(dev->fd, rx_buf, size);
    if (rd != (ssize_t)size) return false;
#endif

    return true;
}

/* Set SPI mode */
bool spi_set_mode(hal_device_id_t device_id, uint8_t mode)
{
    if (device_id >= spi_count) return false;
    spi_t* dev = spi_devices[device_id];
    if (!dev || dev->fd < 0) return false;
    dev->mode = mode;

#ifdef __linux__
    if (ioctl(dev->fd, SPI_IOC_WR_MODE, &mode) < 0) return false;
#elif __FreeBSD__
    // FreeBSD specific mode setting if needed
#endif
    return true;
}

/* Set SPI speed */
bool spi_set_speed(hal_device_id_t device_id, uint32_t speed_hz)
{
    if (device_id >= spi_count) return false;
    spi_t* dev = spi_devices[device_id];
    if (!dev) return false;
    dev->speed_hz = speed_hz;
    return true;
}

/* Set bits per word */
bool spi_set_bits_per_word(hal_device_id_t device_id, uint8_t bits)
{
    if (device_id >= spi_count) return false;
    spi_t* dev = spi_devices[device_id];
    if (!dev || dev->fd < 0) return false;
    dev->bits_per_word = bits;

#ifdef __linux__
    if (ioctl(dev->fd, SPI_IOC_WR_BITS_PER_WORD, &bits) < 0) return false;
#endif
    return true;
}

/* Timeout, flush, drain: no-op / placeholders */
bool spi_set_timeout(hal_device_id_t device_id, uint32_t timeout_ms)
{
    (void)device_id;
    (void)timeout_ms;
    return true;
}

bool spi_flush(hal_device_id_t device_id)
{
    (void)device_id;
    return true;
}

bool spi_drain(hal_device_id_t device_id)
{
    (void)device_id;
    return true;
}

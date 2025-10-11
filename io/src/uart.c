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

#include "uart.h"
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/select.h>

#define MAX_UART_DEVICES MAX_DEVICES

/* Internal state for UART devices */
typedef struct uart_internal_t
{
    hal_device_id_t device_id;
    int fd;
    uint32_t baudrate;
    uart_parity_t parity;
    uint8_t stopbits;
    uint8_t data_bits;
    uart_flow_control_t flow_control;
    uint32_t timeout_ms;
    bool is_open;
} uart_internal_t;

static uart_internal_t uart_table[MAX_UART_DEVICES];

/* Initialize the UART system */
bool uart_init(void)
{
    memset(uart_table, 0, sizeof(uart_table));
    for (size_t i = 0; i < MAX_UART_DEVICES; ++i)
        uart_table[i].device_id = i;
    return true;
}

/* Enumerate devices (stub - platform specific) */
bool uart_enumerate(hal_device_info_t *list, size_t *count)
{
    if (!list || !count) return false;

    /* Stub: return empty list for now */
    *count = 0;
    return true;
}

/* Open a UART device */
bool uart_open(hal_device_id_t device_id)
{
    if (device_id >= MAX_UART_DEVICES) return false;

    uart_internal_t* uart = &uart_table[device_id];
    if (uart->is_open) return true;

    hal_device_info_t dev_info;
    /* TODO: populate dev_info.path from HAL enumeration */
    const char* path = dev_info.path[0] ? dev_info.path : "/dev/ttyS0";

    uart->fd = open(path, O_RDWR | O_NOCTTY | O_NONBLOCK);
    if (uart->fd < 0) return false;

    struct termios tty;
    if (tcgetattr(uart->fd, &tty) != 0)
    {
        close(uart->fd);
        return false;
    }

    /* Set baud rate */
    speed_t speed = B9600; // default
    switch (uart->baudrate)
    {
        case 9600: speed = B9600; break;
        case 19200: speed = B19200; break;
        case 38400: speed = B38400; break;
        case 115200: speed = B115200; break;
        default: speed = B9600; break;
    }
    cfsetispeed(&tty, speed);
    cfsetospeed(&tty, speed);

    /* Set data bits */
    tty.c_cflag &= ~CSIZE;
    switch (uart->data_bits)
    {
        case 5: tty.c_cflag |= CS5; break;
        case 6: tty.c_cflag |= CS6; break;
        case 7: tty.c_cflag |= CS7; break;
        case 8:
        default: tty.c_cflag |= CS8; break;
    }

    /* Set parity */
    switch (uart->parity)
    {
        case HAL_PARITY_ODD:
            tty.c_cflag |= PARENB | PARODD;
            break;
        case HAL_PARITY_EVEN:
            tty.c_cflag |= PARENB;
            tty.c_cflag &= ~PARODD;
            break;
        case HAL_PARITY_NONE:
        default:
            tty.c_cflag &= ~PARENB;
            break;
    }

    /* Stop bits */
    if (uart->stopbits == 2) tty.c_cflag |= CSTOPB;
    else tty.c_cflag &= ~CSTOPB;

    /* Flow control */
    if (uart->flow_control == HAL_FLOW_CONTROL_RTS_CTS) tty.c_cflag |= CRTSCTS;
    else tty.c_cflag &= ~CRTSCTS;

    /* Raw mode */
    tty.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
    tty.c_iflag &= ~(IXON | IXOFF | IXANY | ICRNL);
    tty.c_oflag &= ~OPOST;

    /* Timeout settings */
    tty.c_cc[VMIN] = 0;
    tty.c_cc[VTIME] = uart->timeout_ms / 100; // tenths of a second

    if (tcsetattr(uart->fd, TCSANOW, &tty) != 0)
    {
        close(uart->fd);
        return false;
    }

    uart->is_open = true;
    return true;
}

/* Close UART */
bool uart_close(hal_device_id_t device_id)
{
    if (device_id >= MAX_UART_DEVICES) return false;
    uart_internal_t* uart = &uart_table[device_id];
    if (!uart->is_open) return false;
    close(uart->fd);
    uart->fd = -1;
    uart->is_open = false;
    return true;
}

/* Read from UART with timeout (ms) */
bool uart_read(hal_device_id_t device_id, void* buffer, size_t size)
{
    if (device_id >= MAX_UART_DEVICES || !buffer || size == 0) return false;
    uart_internal_t* uart = &uart_table[device_id];
    if (!uart->is_open) return false;

    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(uart->fd, &readfds);

    struct timeval tv;
    tv.tv_sec = uart->timeout_ms / 1000;
    tv.tv_usec = (uart->timeout_ms % 1000) * 1000;

    int rv = select(uart->fd + 1, &readfds, NULL, NULL, &tv);
    if (rv > 0 && FD_ISSET(uart->fd, &readfds))
    {
        ssize_t n = read(uart->fd, buffer, size);
        return n > 0;
    }

    return false; /* timeout */
}

/* Write to UART */
bool uart_write(hal_device_id_t device_id, const void* data, size_t size)
{
    if (device_id >= MAX_UART_DEVICES || !data || size == 0) return false;
    uart_internal_t* uart = &uart_table[device_id];
    if (!uart->is_open) return false;

    ssize_t n = write(uart->fd, data, size);
    return n == (ssize_t)size;
}

/* Flush input/output */
bool uart_flush(hal_device_id_t device_id)
{
    if (device_id >= MAX_UART_DEVICES) return false;
    uart_internal_t* uart = &uart_table[device_id];
    if (!uart->is_open) return false;
    return tcflush(uart->fd, TCIOFLUSH) == 0;
}

/* Drain output buffer */
bool uart_drain(hal_device_id_t device_id)
{
    if (device_id >= MAX_UART_DEVICES) return false;
    uart_internal_t* uart = &uart_table[device_id];
    if (!uart->is_open) return false;
    return tcdrain(uart->fd) == 0;
}

/* Setters */
bool uart_set_baudrate(hal_device_id_t device_id, uint32_t baudrate)
{
    if (device_id >= MAX_UART_DEVICES) return false;
    uart_internal_t* uart = &uart_table[device_id];
    uart->baudrate = baudrate;
    return uart->is_open ? uart_open(device_id) : true; // reapply settings if open
}

bool uart_set_parity(hal_device_id_t device_id, bool enable, uart_parity_t parity)
{
    if (device_id >= MAX_UART_DEVICES) return false;
    uart_internal_t* uart = &uart_table[device_id];
    uart->parity = enable ? parity : HAL_PARITY_NONE;
    return uart->is_open ? uart_open(device_id) : true;
}

bool uart_set_stopbits(hal_device_id_t device_id, uint8_t stopbits)
{
    if (device_id >= MAX_UART_DEVICES) return false;
    uart_internal_t* uart = &uart_table[device_id];
    uart->stopbits = stopbits;
    return uart->is_open ? uart_open(device_id) : true;
}

bool uart_set_flow_control(hal_device_id_t device_id, bool enable, uart_flow_control_t flow_control)
{
    if (device_id >= MAX_UART_DEVICES) return false;
    uart_internal_t* uart = &uart_table[device_id];
    uart->flow_control = enable ? flow_control : HAL_FLOW_CONTROL_NONE;
    return uart->is_open ? uart_open(device_id) : true;
}

bool uart_set_data_bits(hal_device_id_t device_id, uint8_t data_bits)
{
    if (device_id >= MAX_UART_DEVICES) return false;
    uart_internal_t* uart = &uart_table[device_id];
    uart->data_bits = data_bits;
    return uart->is_open ? uart_open(device_id) : true;
}

bool uart_set_timeout(hal_device_id_t device_id, uint32_t timeout_ms)
{
    if (device_id >= MAX_UART_DEVICES) return false;
    uart_internal_t* uart = &uart_table[device_id];
    uart->timeout_ms = timeout_ms;
    return true;
}

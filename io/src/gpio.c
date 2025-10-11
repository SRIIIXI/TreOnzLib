 
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

#include "gpio.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>

typedef enum gpio_direction_t
{
    GPIO_DIRECTION_IN,
    GPIO_DIRECTION_OUT
} gpio_direction_t;

typedef struct gpio_dev_t
{
    hal_device_id_t device_id;
    char path[256];
    int fd;
    gpio_direction_t direction;
} gpio_dev_t;

#define MAX_GPIO_DEVICES 64
static gpio_dev_t gpio_devices[MAX_GPIO_DEVICES];
static size_t gpio_device_count = 0;

bool gpio_init(void)
{
    gpio_device_count = 0;
    memset(gpio_devices, 0, sizeof(gpio_devices));
    return true;
}

bool gpio_enumerate(hal_device_info_t *list, size_t *count)
{
    if (!list || !count) return false;

    gpio_device_count = 0;

    // Linux typical GPIO sysfs path
    for (int i = 0; i < 32 && gpio_device_count < MAX_GPIO_DEVICES; i++)
    {
        char path[256];
        snprintf(path, sizeof(path), "/sys/class/gpio/gpio%d/value", i);
        if (access(path, F_OK) == 0)
        {
            gpio_devices[gpio_device_count].device_id = (hal_device_id_t)gpio_device_count;
            strncpy(gpio_devices[gpio_device_count].path, path, sizeof(path));
            gpio_devices[gpio_device_count].fd = -1;

            list[gpio_device_count].device_id = gpio_devices[gpio_device_count].device_id;
            list[gpio_device_count].type = HAL_DEVICE_TYPE_GPIO;
            strncpy(list[gpio_device_count].path, path, sizeof(list[gpio_device_count].path));
            snprintf(list[gpio_device_count].name, sizeof(list[gpio_device_count].name), "GPIO_%d", i);
            list[gpio_device_count].capabilities = HAL_CAP_GPIO;
            list[gpio_device_count].metadata = NULL;

            gpio_device_count++;
        }
    }

    *count = gpio_device_count;
    return true;
}

bool gpio_open(hal_device_id_t device_id)
{
    if (device_id >= gpio_device_count) return false;

    int fd = open(gpio_devices[device_id].path, O_RDWR);
    if (fd < 0) return false;

    gpio_devices[device_id].fd = fd;
    return true;
}

bool gpio_close(hal_device_id_t device_id)
{
    if (device_id >= gpio_device_count) return false;
    if (gpio_devices[device_id].fd >= 0) close(gpio_devices[device_id].fd);
    gpio_devices[device_id].fd = -1;
    return true;
}

bool gpio_set_direction(hal_device_id_t device_id, gpio_direction_t direction)
{
    if (device_id >= gpio_device_count) return false;
    gpio_devices[device_id].direction = direction;

    char path[256];
    snprintf(path, sizeof(path), "/sys/class/gpio/gpio%d/direction", device_id);
    int fd = open(path, O_WRONLY);
    if (fd < 0) return false;

    const char *dir_str = (direction == GPIO_DIRECTION_OUT) ? "out" : "in";
    write(fd, dir_str, strlen(dir_str));
    close(fd);

    return true;
}

bool gpio_get_direction(hal_device_id_t device_id, gpio_direction_t *direction)
{
    if (device_id >= gpio_device_count || !direction) return false;
    *direction = gpio_devices[device_id].direction;
    return true;
}

bool gpio_write(hal_device_id_t device_id, const void* data, size_t size)
{
    if (device_id >= gpio_device_count) return false;
    int fd = gpio_devices[device_id].fd;
    if (fd < 0) return false;
    if (!data) return false;

    const char c = (((char*)data)[0] != 0) ? '1' : '0';
    lseek(fd, 0, SEEK_SET);
    if (write(fd, &c, 1) != 1) return false;

    return true;
}

bool gpio_read(hal_device_id_t device_id, void* buffer, size_t size)
{
    if (device_id >= gpio_device_count || !buffer) return false;
    int fd = gpio_devices[device_id].fd;
    if (fd < 0) return false;

    char c;
    lseek(fd, 0, SEEK_SET);
    if (read(fd, &c, 1) != 1) return false;

    ((char*)buffer)[0] = (c == '1') ? 1 : 0;
    return true;
}

bool gpio_toggle(hal_device_id_t device_id)
{
    if (device_id >= gpio_device_count) return false;

    char c[2] = {0};
    if (!gpio_read(device_id, &c, 1)) return false;
    c[0] = (c[0] == 1) ? 0 : 1;
    return gpio_write(device_id, &c, 1);
}

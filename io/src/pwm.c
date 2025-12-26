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

#include "pwm.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <fcntl.h>
#include <unistd.h>

#ifdef __linux__
#include <dirent.h>
#endif

#ifdef __FreeBSD__
#include <sys/ioctl.h>
#include <pwm/pwmc.h>
#endif

#ifndef HAL_MAX_DEVICES
#define HAL_MAX_DEVICES 8
#endif

#ifdef __linux__
#define SYSFS_PWM_PATH "/sys/class/pwm"
#endif

// -----------------------------------------------------------------------------
// Local opaque structure
// -----------------------------------------------------------------------------

typedef struct pwm_device_t
{
#ifdef __linux__
    int fd_enable;
    int fd_period;
    int fd_duty_cycle;
    int fd_polarity;
#elif __FreeBSD__
    int fd; // single /dev/pwmN
#endif
    bool opened;
} pwm_device_t;

static pwm_device_t pwm_devices[HAL_MAX_DEVICES];
static size_t pwm_device_count = 0;

// -----------------------------------------------------------------------------
// File helpers for Linux
// -----------------------------------------------------------------------------

#ifdef __linux__
static int pwm_fd_open(const char *path, int flags)
{
    int fd = open(path, flags);
    if (fd < 0)
    {
        perror(path);
    }
    return fd;
}

static bool pwm_fd_write(int fd, const char *val)
{
    if (fd < 0)
    {
        return false;
    }
    ssize_t n = write(fd, val, strlen(val));
    return n == (ssize_t)strlen(val);
}

static bool pwm_fd_read(int fd, char *buf, size_t size)
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
// Initialize PWM
// -----------------------------------------------------------------------------

bool pwm_init(void)
{
    memset(pwm_devices, 0, sizeof(pwm_devices));
    pwm_device_count = 0;

#ifdef __linux__
    DIR *dp = opendir(SYSFS_PWM_PATH);
    if (!dp)
    {
        perror("opendir pwmchip");
        return false;
    }

    struct dirent *entry;
    while ((entry = readdir(dp)) != NULL && pwm_device_count < HAL_MAX_DEVICES)
    {
        if (strncmp(entry->d_name, "pwmchip", 7) != 0)
        {
            continue;
        }

        char export_path[256];
        snprintf(export_path, sizeof(export_path), "%s/%s/export", SYSFS_PWM_PATH, entry->d_name);

        int fd = pwm_fd_open(export_path, O_WRONLY);
        if (fd >= 0)
        {
            pwm_fd_write(fd, "0"); // export PWM 0
            close(fd);
        }

        char base[256];
        snprintf(base, sizeof(base), "%s/%s/pwm0", SYSFS_PWM_PATH, entry->d_name);

        pwm_device_t *dev = &pwm_devices[pwm_device_count];

        char path[256];
        snprintf(path, sizeof(path), "%s/enable", base);
        dev->fd_enable = pwm_fd_open(path, O_RDWR);

        snprintf(path, sizeof(path), "%s/period", base);
        dev->fd_period = pwm_fd_open(path, O_RDWR);

        snprintf(path, sizeof(path), "%s/duty_cycle", base);
        dev->fd_duty_cycle = pwm_fd_open(path, O_RDWR);

        snprintf(path, sizeof(path), "%s/polarity", base);
        dev->fd_polarity = pwm_fd_open(path, O_RDWR);

        dev->opened = true;
        pwm_device_count++;
    }

    closedir(dp);
#elif __FreeBSD__
    for (int i = 0; i < HAL_MAX_DEVICES; i++)
    {
        char path[64];
        snprintf(path, sizeof(path), "/dev/pwm%d", i);
        int fd = open(path, O_RDWR);
        if (fd >= 0)
        {
            pwm_devices[pwm_device_count].fd = fd;
            pwm_devices[pwm_device_count].opened = true;
            pwm_device_count++;
        }
    }
#endif

    return pwm_device_count > 0;
}

// -----------------------------------------------------------------------------
// Enumerate PWM devices
// -----------------------------------------------------------------------------

bool pwm_enumerate(hal_device_info_t *list, size_t *count)
{
    if (!list || !count)
    {
        return false;
    }

    size_t n = (*count < pwm_device_count) ? *count : pwm_device_count;
    for (size_t i = 0; i < n; i++)
    {
        snprintf(list[i].path, sizeof(list[i].path), "/dev/pwm%zu", i);
        snprintf(list[i].name, sizeof(list[i].name), "PWM_Device_%zu", i);
        list[i].type = HAL_DEVICE_TYPE_PWM;
        list[i].capabilities = HAL_CAP_PWM;
        list[i].metadata = NULL;
    }
    *count = n;
    return true;
}

// -----------------------------------------------------------------------------
// Frequency
// -----------------------------------------------------------------------------

bool pwm_set_frequency(hal_device_id_t device_id, void *config, size_t size)
{
    (void)size;
    if (device_id >= pwm_device_count || !config)
    {
        return false;
    }

    uint32_t hz = *((uint32_t *)config);
    if (hz == 0)
    {
        return false;
    }

#ifdef __linux__
    uint64_t period_ns = 1000000000ULL / hz;
    char buf[32];
    snprintf(buf, sizeof(buf), "%ld", period_ns);
   return pwm_fd_write(pwm_devices[device_id].fd_period, buf);
#elif __FreeBSD__
    struct pwm_state state;
    int fd = pwm_devices[device_id].fd;

    // 1. You MUST get the current state first to preserve the duty cycle and enable flag
    if (ioctl(fd, PWMGETSTATE, &state) == -1) {
        return false;
    }

    // 2. Update the period (FreeBSD uses 'period' field in nanoseconds)
    state.period = 1000000000ULL / hz;

    // 3. Apply the updated state
    return ioctl(fd, PWMSETSTATE, &state) == 0;
#endif
}

bool pwm_get_frequency(hal_device_id_t device_id, void *config, size_t size)
{
    (void)size;
    if (device_id >= pwm_device_count || !config)
    {
        return false;
    }

#ifdef __linux__
    char buf[32];
    if (!pwm_fd_read(pwm_devices[device_id].fd_period, buf, sizeof(buf)))
    {
        return false;
    }
    uint64_t period_ns = strtoull(buf, NULL, 10);
    *((uint32_t *)config) = (uint32_t)(1000000000ULL / period_ns);
    return true;
#elif __FreeBSD__
    struct pwm_state state;
    if (ioctl(pwm_devices[device_id].fd, PWMGETSTATE, &state) != 0)
    {
        return false;
    }
    *((uint32_t *)config) = (uint32_t)(1000000000ULL / state.period);
    return true;
#endif
}

// -----------------------------------------------------------------------------
// Duty cycle
// -----------------------------------------------------------------------------

bool pwm_set_duty_cycle(hal_device_id_t device_id, void *config, size_t size)
{
    (void)size;
    if (device_id >= pwm_device_count || !config)
    {
        return false;
    }

    float duty = *((float *)config);
    if (duty < 0.0f || duty > 1.0f)
    {
        return false;
    }

#ifdef __linux__
    char buf[32];
    char period_buf[32];
    if (!pwm_fd_read(pwm_devices[device_id].fd_period, period_buf, sizeof(period_buf)))
    {
        return false;
    }
    uint64_t period_ns = strtoull(period_buf, NULL, 10);
    uint64_t duty_ns = (uint64_t)(duty * period_ns);
    snprintf(buf, sizeof(buf), "%ld", duty_ns);
    return pwm_fd_write(pwm_devices[device_id].fd_duty_cycle, buf);
#elif __FreeBSD__
    struct pwm_state state;
    int fd = pwm_devices[device_id].fd;

    // 1. Fetch current period and status
    if (ioctl(fd, PWMGETSTATE, &state) != 0) {
        return false;
    }

    // 2. Calculate duty cycle
    // FreeBSD .duty and .period are both in nanoseconds.
    // If 'duty' is a float/double percentage (0.0 to 1.0):
    state.duty = (uint32_t)((double)duty * state.period);

    // 3. Set the updated state
    return ioctl(fd, PWMSETSTATE, &state) == 0;
#endif
}

bool pwm_get_duty_cycle(hal_device_id_t device_id, void *config, size_t size)
{
    (void)size;
    if (device_id >= pwm_device_count || !config)
    {
        return false;
    }

#ifdef __linux__
    char buf[32];
    char period_buf[32];
    if (!pwm_fd_read(pwm_devices[device_id].fd_duty_cycle, buf, sizeof(buf)))
    {
        return false;
    }
    if (!pwm_fd_read(pwm_devices[device_id].fd_period, period_buf, sizeof(period_buf)))
    {
        return false;
    }
    uint64_t duty_ns = strtoull(buf, NULL, 10);
    uint64_t period_ns = strtoull(period_buf, NULL, 10);
    *((float *)config) = (float)duty_ns / (float)period_ns;
    return true;
#elif __FreeBSD__
    struct pwm_state state;
    int fd = pwm_devices[device_id].fd;

    // FreeBSD fetches both period and duty in a single call
    if (ioctl(fd, PWMGETSTATE, &state) != 0)
    {
        return false;
    }

    // Calculate the ratio
    // .duty and .period are uint32_t values in nanoseconds
    if (state.period > 0) {
        *((float *)config) = (float)state.duty / (float)state.period;
    } else {
        *((float *)config) = 0.0f;
    }

    return true;
#endif
}

// -----------------------------------------------------------------------------
// Polarity
// -----------------------------------------------------------------------------

bool pwm_set_polarity(hal_device_id_t device_id, void *config, size_t size)
{
    (void)size;
    if (device_id >= pwm_device_count || !config)
    {
        return false;
    }
    bool normal = *((bool *)config);

#ifdef __linux__
    return pwm_fd_write(pwm_devices[device_id].fd_polarity, normal ? "normal" : "inversed");
#elif __FreeBSD__
    struct pwm_state state;
    int fd = pwm_devices[device_id].fd;

    // 1. Fetch current state
    if (ioctl(fd, PWMGETSTATE, &state) != 0) {
        return false;
    }

    // 2. Set/Clear the inverted flag
    // FreeBSD defines PWM_POLARITY_INVERTED in <dev/pwm/pwmc.h>
    if (normal) {
        state.flags &= ~PWM_POLARITY_INVERTED;
    } else {
        state.flags |= PWM_POLARITY_INVERTED;
    }

    // 3. Apply the updated state
    return ioctl(fd, PWMSETSTATE, &state) == 0;
#endif
}

bool pwm_get_polarity(hal_device_id_t device_id, void *config, size_t size)
{
    (void)size;
    if (device_id >= pwm_device_count || !config)
    {
        return false;
    }

#ifdef __linux__
    char buf[16];
    if (!pwm_fd_read(pwm_devices[device_id].fd_polarity, buf, sizeof(buf)))
    {
        return false;
    }
    *((bool *)config) = (strncmp(buf, "normal", 6) == 0);
    return true;
#elif __FreeBSD__
    struct pwm_state state;
    int fd = pwm_devices[device_id].fd;

    // 1. Fetch the unified state
    if (ioctl(fd, PWMGETSTATE, &state) != 0)
    {
        return false;
    }

    // 2. Check the inversion bit in the flags field
    // In FreeBSD, 'normal' is the absence of the PWM_POLARITY_INVERTED flag.
    *((bool *)config) = !(state.flags & PWM_POLARITY_INVERTED);

    return true;
#endif
}

// -----------------------------------------------------------------------------
// Enable / Disable
// -----------------------------------------------------------------------------

bool pwm_enable(hal_device_id_t device_id)
{
    if (device_id >= pwm_device_count)
    {
        return false;
    }
#ifdef __linux__
    return pwm_fd_write(pwm_devices[device_id].fd_enable, "1");
#elif __FreeBSD__
    struct pwm_state state;
    int fd = pwm_devices[device_id].fd;

    // 1. Fetch current state to preserve period, duty, and flags
    if (ioctl(fd, PWMGETSTATE, &state) != 0) {
        return false;
    }

    // 2. Set the enable flag
    // val == 1 (enable), val == 0 (disable)
    bool val = true;
    state.enable = (bool)val;

    // 3. Write the state back to the hardware
    return ioctl(fd, PWMSETSTATE, &state) == 0;
#endif
}

bool pwm_disable(hal_device_id_t device_id)
{
    if (device_id >= pwm_device_count)
    {
        return false;
    }
#ifdef __linux__
    return pwm_fd_write(pwm_devices[device_id].fd_enable, "0");
#elif __FreeBSD__
    struct pwm_state state;
    int fd = pwm_devices[device_id].fd;

    // 1. Fetch current state to preserve period, duty, and flags
    if (ioctl(fd, PWMGETSTATE, &state) != 0) {
        return false;
    }

    // 2. Set the enable flag
    // val == 1 (enable), val == 0 (disable)
    bool val = false;
    state.enable = (bool)val;

    // 3. Write the state back to the hardware
    return ioctl(fd, PWMSETSTATE, &state) == 0;
#endif
}

bool pwm_is_enabled(hal_device_id_t device_id, void *config, size_t size)
{
    (void)size;
    if (device_id >= pwm_device_count || !config)
    {
        return false;
    }
#ifdef __linux__
    char buf[4];
    if (!pwm_fd_read(pwm_devices[device_id].fd_enable, buf, sizeof(buf)))
    {
        return false;
    }
    *((bool *)config) = (strncmp(buf, "1", 1) == 0);
    return true;
#elif __FreeBSD__
    struct pwm_state state;
    int fd = pwm_devices[device_id].fd;

    // 1. Fetch current state to preserve period, duty, and flags
    if (ioctl(fd, PWMGETSTATE, &state) != 0) {
        return false;
    }
    return state.enable;
#endif
}

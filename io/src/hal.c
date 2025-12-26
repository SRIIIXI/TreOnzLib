 
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

#include "hal.h"
#include "uart.h"
#include "spi.h"
#include "i2c.h"
#include "pwm.h"
#include "gpio.h"
#include "adc.h"
#include "can.h"

static hal_device_info_t device_registry[MAX_DEVICES];
static size_t device_count = 0;
static bool hal_initialized = false;


bool hal_open(void)
{
    if (!hal_initialized)
    {
        return false;
    }

    int status = 0;

    status |= uart_init();
    status |= spi_init();
    status |= i2c_init();
    status |= i2c_init();
    status |= pwm_init();
    status |= gpio_init();
    status |= adc_init();

#ifdef __linux__
    status |= can_init();
#endif

    if (status == 0)
    {
        hal_initialized = true;
    }

    return status;
}

bool hal_close(void)
{
    // Optional: driver-specific shutdowns

    hal_initialized = false;
    device_count = 0;
    memset(device_registry, 0, sizeof(device_registry));

    return true;
}

bool hal_enumerate(hal_device_info_t *device_list, size_t *num_devices)
{
    if (!hal_initialized || !device_list || !num_devices)
    {
        return false;
    }

    size_t total = 0;
    size_t count;

    // UART
    count = MAX_DEVICES - total;
    if (uart_enumerate(&device_list[total], &count) == 0)
    {
        total += count;
    }

    // SPI
    count = MAX_DEVICES - total;
    if (spi_enumerate(&device_list[total], &count) == 0)
    {
        total += count;
    }

    // I2C
    count = MAX_DEVICES - total;
    if (i2c_enumerate(&device_list[total], &count) == 0)
    {
        total += count;
    }

     // ADC
    count = MAX_DEVICES - total;
    if (adc_enumerate(&device_list[total], &count) == 0)
    {
        total += count;
    }

    // GPIO
    count = MAX_DEVICES - total;
    if (gpio_enumerate(&device_list[total], &count) == 0)
    {
        total += count;
    }

    // PWM
    count = MAX_DEVICES - total;
    if (pwm_enumerate(&device_list[total], &count) == 0)
    {
        total += count;
    }


#ifdef __linux__    // CAN
    count = MAX_DEVICES - total;            
    if (can_enumerate(&device_list[total], &count) == 0)
    {
        total += count;
    }
#endif

    memcpy(device_registry, device_list, total * sizeof(hal_device_info_t));
    device_count = total;
    *num_devices = total;

    return true;
}

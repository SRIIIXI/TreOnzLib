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

bool uart_init(void)
{
    return false;
}

bool uart_enumerate(hal_device_info_t *list, size_t *count)
{
    return false;
}

bool uart_open(hal_device_id_t device_id)
{
    return false;
}

bool uart_close(hal_device_id_t device_id)
{
    return false;
}

bool uart_read(hal_device_id_t device_id, void* buffer, size_t size)
{
    return false;
}

bool uart_write(hal_device_id_t device_id, const void* data, size_t size)
{
    return false;
}

bool uart_set_baudrate(hal_device_id_t device_id, uint32_t baudrate)
{
    return false;
}

bool uart_set_parity(hal_device_id_t device_id, bool enable, uart_parity_t parity)
{
    return false;
}

bool uart_set_stopbits(hal_device_id_t device_id, uint8_t stopbits)
{
    return false;
}

bool uart_set_flow_control(hal_device_id_t device_id, bool enable, uart_flow_control_t flow_control)
{
    return false;
}

bool uart_set_data_bits(hal_device_id_t device_id, uint8_t data_bits)
{
    return false;
}

bool uart_set_timeout(hal_device_id_t device_id, uint32_t timeout_ms)
{
    return false;
}

bool uart_flush(hal_device_id_t device_id)
{
    return false;
}

bool uart_drain(hal_device_id_t device_id)
{
    return false;
}


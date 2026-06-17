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

#ifndef SPI_H
#define SPI_H

#include "defines.h"
#include "haltypes.h"
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

extern LIBRARY_EXPORT bool spi_init(void);  
extern LIBRARY_EXPORT bool spi_enumerate(hal_device_info_t *list, size_t *count);
extern LIBRARY_EXPORT bool spi_open(hal_device_id_t device_id);
extern LIBRARY_EXPORT bool spi_close(hal_device_id_t device_id);
extern LIBRARY_EXPORT bool spi_read(hal_device_id_t device_id, void* buffer, size_t size);
extern LIBRARY_EXPORT bool spi_write(hal_device_id_t device_id, const void* data, size_t size);
extern LIBRARY_EXPORT bool spi_set_mode(hal_device_id_t device_id, uint8_t mode);
extern LIBRARY_EXPORT bool spi_set_speed(hal_device_id_t device_id, uint32_t speed_hz);
extern LIBRARY_EXPORT bool spi_set_bits_per_word(hal_device_id_t device_id, uint8_t bits);
extern LIBRARY_EXPORT bool spi_set_timeout(hal_device_id_t device_id, uint32_t timeout_ms);
extern LIBRARY_EXPORT bool spi_flush(hal_device_id_t device_id);
extern LIBRARY_EXPORT bool spi_drain(hal_device_id_t device_id);


#ifdef __cplusplus
}
#endif

#endif // SPI_H

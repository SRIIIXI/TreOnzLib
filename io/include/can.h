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

#ifndef CAN_H
#define CAN_H

#include "defines.h"
#include "haltypes.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// Initialize CAN subsystem
extern LIBRARY_EXPORT bool can_init(void);

// Enumerate CAN devices
extern LIBRARY_EXPORT bool can_enumerate(hal_device_info_t *list, size_t *count);

// Send a CAN frame
extern LIBRARY_EXPORT bool can_write(hal_device_id_t device_id, void *frame, size_t size);

// Receive a CAN frame
extern LIBRARY_EXPORT bool can_read(hal_device_id_t device_id, void *frame, size_t size, int timeout_ms);

// Set bitrate (config opaque pointer)
extern LIBRARY_EXPORT bool can_set_bitrate(hal_device_id_t device_id, void *config, size_t size);

// Get bitrate
extern LIBRARY_EXPORT bool can_get_bitrate(hal_device_id_t device_id, void *config, size_t size);

// Get CAN state (active, error, bus off)
extern LIBRARY_EXPORT bool can_get_state(hal_device_id_t device_id, void *config, size_t size);

#ifdef __cplusplus
}
#endif

#endif // CAN_H

 
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

#ifndef HAL_H
#define HAL_H

#include "defines.h"
#include "haltypes.h"
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

// Initialization and teardown
extern LIBRARY_EXPORT bool hal_open(void);     // Initialize all hardware interfaces
extern LIBRARY_EXPORT bool hal_close(void);    // Shutdown and cleanup

// Device enumeration
extern LIBRARY_EXPORT bool hal_enumerate(hal_device_info_t *device_list, size_t *num_devices);

// I/O operations
extern LIBRARY_EXPORT bool hal_read(hal_device_id_t device_id, void* buffer, size_t size);
extern LIBRARY_EXPORT bool hal_write(hal_device_id_t device_id, const void* data, size_t size);

// Device control
extern LIBRARY_EXPORT bool hal_enable(hal_device_id_t device_id);   // Enable device (e.g., power up, activate)
extern LIBRARY_EXPORT bool hal_disable(hal_device_id_t device_id);  // Disable device (e.g., power down, deactivate)

// Event registration
extern LIBRARY_EXPORT bool hal_register_callback(hal_device_id_t device_id, hal_event_callback_t callback);

#ifdef __cplusplus
}
#endif

#endif // HAL_H

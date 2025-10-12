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

#ifndef PWM_H
#define PWM_H

#include "defines.h"
#include "haltypes.h"
#include <string.h>
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// -----------------------------------------------------------------------------
// PWM HAL API
// -----------------------------------------------------------------------------

extern LIBRARY_EXPORT bool pwm_init(void);
extern LIBRARY_EXPORT bool pwm_enumerate(hal_device_info_t *list, size_t *count);

// Frequency in Hz
extern LIBRARY_EXPORT bool pwm_set_frequency(hal_device_id_t device_id, void *config, size_t size);
extern LIBRARY_EXPORT bool pwm_get_frequency(hal_device_id_t device_id, void *config, size_t size);

// Duty cycle 0.0f - 1.0f
extern LIBRARY_EXPORT bool pwm_set_duty_cycle(hal_device_id_t device_id, void *config, size_t size);
extern LIBRARY_EXPORT bool pwm_get_duty_cycle(hal_device_id_t device_id, void *config, size_t size);

// Polarity (true = normal, false = inversed)
extern LIBRARY_EXPORT bool pwm_set_polarity(hal_device_id_t device_id, void *config, size_t size);
extern LIBRARY_EXPORT bool pwm_get_polarity(hal_device_id_t device_id, void *config, size_t size);

// Enable / Disable PWM
extern LIBRARY_EXPORT bool pwm_enable(hal_device_id_t device_id);
extern LIBRARY_EXPORT bool pwm_disable(hal_device_id_t device_id);
extern LIBRARY_EXPORT bool pwm_is_enabled(hal_device_id_t device_id, void *config, size_t size);

#ifdef __cplusplus
}
#endif

#endif // PWM_H

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

#ifndef DEVICE_BASE_H
#define DEVICE_BASE_H

#include "types.h"

/* Must be the FIRST member of every struct xf_device definition so that
 * shared functions can cast xf_device_t* to xf_device_base_t* safely. */
typedef struct {
    int              fd;
    xf_orientation_t orientation;
    int              display_width;
    int              display_height;
} xf_device_base_t;

#endif /* DEVICE_BASE_H */

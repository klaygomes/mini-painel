#ifndef TURING_DEVICE_INTERNAL_H
#define TURING_DEVICE_INTERNAL_H

#include "device_base.h"

/* TURING_3_5 is the official Turing Smart Screen 3.5"; it does not answer HELLO. */
typedef enum {
    TURING_VARIANT_UNKNOWN = 0,
    TURING_VARIANT_TURING_3_5,
    TURING_VARIANT_USBMONITOR_3_5,
    TURING_VARIANT_USBMONITOR_5,
    TURING_VARIANT_USBMONITOR_7,
} turing_variant_t;

struct xf_device {
    xf_device_base_t  base;           /* must stay first */
    turing_variant_t  variant;
};

#endif /* TURING_DEVICE_INTERNAL_H */

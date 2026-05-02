#ifndef DEVICE_INTERNAL_H
#define DEVICE_INTERNAL_H

#include "device_base.h"

/* The name encodes the two HELLO response bytes: byte[6]=0x0A, byte[7]=0x01/02/11/12. */
typedef enum {
    XF_SUB_REV_UNKNOWN = 0,
    XF_SUB_REV_A01,
    XF_SUB_REV_A02,
    XF_SUB_REV_A11,
    XF_SUB_REV_A12
} xf_sub_revision_t;

/* Include this header only in panel.c and tests that need direct field access. */
struct xf_device {
    xf_device_base_t  base;           /* must stay first */
    xf_sub_revision_t sub_revision;
    uint8_t          *img_scratch;    /* display_width * display_height * 3, rotation workspace */
    uint8_t          *img_rgb565;     /* display_width * display_height * 2, RGB565 workspace */
};

#endif /* DEVICE_INTERNAL_H */

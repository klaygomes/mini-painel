#ifndef PANEL_TYPES_H
#define PANEL_TYPES_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

/* REVERSE_* orientations are software-managed (180° pixel flip before send). */
typedef enum {
    XF_ORIENT_PORTRAIT = 0,
    XF_ORIENT_LANDSCAPE,
    XF_ORIENT_REVERSE_PORTRAIT,
    XF_ORIENT_REVERSE_LANDSCAPE
} xf_orientation_t;

typedef struct {
    uint8_t r;
    uint8_t g;
    uint8_t b;
} xf_color_t;

typedef struct xf_device xf_device_t;

#endif /* PANEL_TYPES_H */

#ifndef PANEL_TYPES_H
#define PANEL_TYPES_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

/* Values match the Turing protocol: device byte = value + 100. */
typedef enum {
    XF_ORIENT_PORTRAIT          = 0,
    XF_ORIENT_REVERSE_PORTRAIT  = 1,
    XF_ORIENT_LANDSCAPE         = 2,
    XF_ORIENT_REVERSE_LANDSCAPE = 3
} xf_orientation_t;

typedef struct {
    uint8_t r;
    uint8_t g;
    uint8_t b;
} xf_color_t;

typedef struct xf_device xf_device_t;

#endif /* PANEL_TYPES_H */

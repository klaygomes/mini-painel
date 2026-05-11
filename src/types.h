/**
 * @file types.h
 * @brief Shared public types used across the panel, dashboard, and components.
 */

#ifndef PANEL_TYPES_H
#define PANEL_TYPES_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

/**
 * @brief Display orientation values matching the Turing hardware protocol.
 *
 * The device byte is sent as the enum value plus 100.
 */
typedef enum {
    XF_ORIENT_PORTRAIT          = 0,
    XF_ORIENT_REVERSE_PORTRAIT  = 1,
    XF_ORIENT_LANDSCAPE         = 2,
    XF_ORIENT_REVERSE_LANDSCAPE = 3
} xf_orientation_t;

/**
 * @brief 8-bit RGB colour.
 */
typedef struct {
    uint8_t r; /**< Red channel.   */
    uint8_t g; /**< Green channel. */
    uint8_t b; /**< Blue channel.  */
} xf_color_t;

/**
 * @brief Opaque device handle returned by panel_open().
 */
typedef struct xf_device xf_device_t;

typedef struct {
    int            x;
    int            y;
    int            width;
    int            height;
    const uint8_t *rgb888;
} xf_bitmap_t;

#endif /* PANEL_TYPES_H */

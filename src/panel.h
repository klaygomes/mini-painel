#ifndef PANEL_H
#define PANEL_H

#include "types.h"

xf_device_t *panel_open(const char *port);
xf_device_t *panel_open_auto(void);
void          panel_close(xf_device_t *dev);

bool panel_is_flagship(const xf_device_t *dev);
bool panel_is_brightness_range(const xf_device_t *dev);

/* REVERSE_* orientations are software-managed: hardware receives the base
 * orientation and pixels are rotated 180° in software before transmission. */
int panel_set_orientation(xf_device_t *dev, xf_orientation_t orientation);

/* level in [0, 100] percent.
 * Range devices (A11/A12): maps linearly to 0-255.
 * Binary devices (A01/A02): 0 = off, any other value = full brightness. */
int panel_set_brightness(xf_device_t *dev, int level);

/* No-op on non-flagship devices. */
int panel_set_led(xf_device_t *dev, xf_color_t color);

/* The device has no native clear command; this sends a full-screen white bitmap. */
int panel_clear(xf_device_t *dev);

int panel_screen_off(xf_device_t *dev);
int panel_screen_on(xf_device_t *dev);

/* rgb888 is width*height*3 bytes in R,G,B order.
 * For REVERSE_* orientations, coordinates and pixels are automatically adjusted. */
int panel_display_bitmap(xf_device_t *dev,
                         int x, int y,
                         int width, int height,
                         const uint8_t *rgb888);

#endif /* PANEL_H */

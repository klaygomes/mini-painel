#ifndef IMAGE_H
#define IMAGE_H

#include <stdint.h>

/* Formula: px = ((R>>3)<<11) | ((G>>2)<<5) | (B>>3), stored big-endian.
 * out must be pixel_count * 2 bytes. */
void image_rgb888_to_rgb565be(const uint8_t *rgb888, int pixel_count, uint8_t *out);

/* Used for REVERSE_PORTRAIT and REVERSE_LANDSCAPE orientations.
 * in and out must not alias. */
void image_rotate_180(const uint8_t *in, int width, int height, uint8_t *out);

#endif /* IMAGE_H */

#ifndef TURING_IMAGE_H
#define TURING_IMAGE_H

#include <stdint.h>

void turing_image_rgb888_to_rgb565le(const uint8_t *rgb888, int pixel_count, uint8_t *out);

/* in and out must not alias. */
void turing_image_rotate_180(const uint8_t *in, int width, int height, uint8_t *out);

#endif /* TURING_IMAGE_H */

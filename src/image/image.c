#include "image.h"

void image_rgb888_to_rgb565be(const uint8_t *rgb888, int pixel_count, uint8_t *out)
{
    int i;
    for (i = 0; i < pixel_count; i++) {
        uint16_t r = (uint16_t)(rgb888[i * 3 + 0] >> 3);
        uint16_t g = (uint16_t)(rgb888[i * 3 + 1] >> 2);
        uint16_t b = (uint16_t)(rgb888[i * 3 + 2] >> 3);
        uint16_t px = (uint16_t)((r << 11) | (g << 5) | b);
        out[i * 2 + 0] = (uint8_t)((px >> 8) & 0xFF);
        out[i * 2 + 1] = (uint8_t)(px & 0xFF);
    }
}

void image_rotate_180(const uint8_t *in, int width, int height, uint8_t *out)
{
    int total = width * height;
    int i;
    for (i = 0; i < total; i++) {
        int j = total - 1 - i;
        out[j * 3 + 0] = in[i * 3 + 0];
        out[j * 3 + 1] = in[i * 3 + 1];
        out[j * 3 + 2] = in[i * 3 + 2];
    }
}

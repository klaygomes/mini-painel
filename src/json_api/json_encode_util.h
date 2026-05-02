#ifndef XF_JSON_ENCODE_UTIL_H
#define XF_JSON_ENCODE_UTIL_H

#include <stdio.h>
#include "theme.h"

/*
 * Serialise an xf_rgba_t to a "#rrggbbaa" hex string (9 chars + NUL).
 * out must point to a buffer of at least 10 bytes.
 */
static void rgba_to_hex(xf_rgba_t c, char out[10])
{
    unsigned int r = (unsigned int)(c.r * 255.0 + 0.5);
    unsigned int g = (unsigned int)(c.g * 255.0 + 0.5);
    unsigned int b = (unsigned int)(c.b * 255.0 + 0.5);
    unsigned int a = (unsigned int)(c.a * 255.0 + 0.5);
    if (r > 255) r = 255;
    if (g > 255) g = 255;
    if (b > 255) b = 255;
    if (a > 255) a = 255;
    snprintf(out, 10, "#%02x%02x%02x%02x", r, g, b, a);
}

#include "json_enc_buf.h"

#endif /* XF_JSON_ENCODE_UTIL_H */

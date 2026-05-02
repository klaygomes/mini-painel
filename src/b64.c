#include "b64.h"

static const char ALPHABET[] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

int xf_b64_encode(const uint8_t *in, size_t in_len, char *out, size_t cap)
{
    size_t out_len = XF_B64_ENCODE_LEN(in_len);
    size_t i;
    size_t o = 0;

    if (!out || cap < out_len)
        return -1;

    for (i = 0; i + 2 < in_len; i += 3) {
        uint32_t v = ((uint32_t)in[i] << 16) |
                     ((uint32_t)in[i+1] << 8) |
                      (uint32_t)in[i+2];
        out[o++] = ALPHABET[(v >> 18) & 0x3F];
        out[o++] = ALPHABET[(v >> 12) & 0x3F];
        out[o++] = ALPHABET[(v >>  6) & 0x3F];
        out[o++] = ALPHABET[ v        & 0x3F];
    }

    if (i < in_len) {
        uint32_t v = (uint32_t)in[i] << 16;
        if (i + 1 < in_len)
            v |= (uint32_t)in[i+1] << 8;
        out[o++] = ALPHABET[(v >> 18) & 0x3F];
        out[o++] = ALPHABET[(v >> 12) & 0x3F];
        out[o++] = (i + 1 < in_len) ? ALPHABET[(v >> 6) & 0x3F] : '=';
        out[o++] = '=';
    }

    out[o] = '\0';
    return (int)o;
}

#include "b64.h"

static const char ALPHABET[] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

int xf_b64_encode(xf_byte_slice_t in, xf_out_buf_t out)
{
    size_t out_len = XF_B64_ENCODE_LEN(in.len);
    size_t i;
    size_t o = 0;

    if (!out.ptr || out.cap < out_len)
        return -1;

    for (i = 0; i + 2 < in.len; i += 3) {
        uint32_t v = ((uint32_t)in.ptr[i] << 16) |
                     ((uint32_t)in.ptr[i+1] << 8) |
                      (uint32_t)in.ptr[i+2];
        out.ptr[o++] = ALPHABET[(v >> 18) & 0x3F];
        out.ptr[o++] = ALPHABET[(v >> 12) & 0x3F];
        out.ptr[o++] = ALPHABET[(v >>  6) & 0x3F];
        out.ptr[o++] = ALPHABET[ v        & 0x3F];
    }

    if (i < in.len) {
        uint32_t v = (uint32_t)in.ptr[i] << 16;
        if (i + 1 < in.len)
            v |= (uint32_t)in.ptr[i+1] << 8;
        out.ptr[o++] = ALPHABET[(v >> 18) & 0x3F];
        out.ptr[o++] = ALPHABET[(v >> 12) & 0x3F];
        out.ptr[o++] = (i + 1 < in.len) ? ALPHABET[(v >> 6) & 0x3F] : '=';
        out.ptr[o++] = '=';
    }

    out.ptr[o] = '\0';
    return (int)o;
}

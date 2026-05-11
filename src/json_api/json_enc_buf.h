#ifndef XF_JSON_ENC_BUF_H
#define XF_JSON_ENC_BUF_H

#include <string.h>
#include "buf.h"

/*
 * Buffer-append helper used by encode_*.c functions that build JSON manually.
 * Requires local variables in scope: xf_out_buf_t out, size_t pos.
 * Returns -1 from the calling function on overflow.
 */
#define XF_ENC_APPEND(s, len) do { \
    size_t _l = (size_t)(len); \
    if (pos + _l + 1 > out.cap) return -1; \
    memcpy(out.ptr + pos, (s), _l); pos += _l; out.ptr[pos] = '\0'; \
} while (0)

#endif /* XF_JSON_ENC_BUF_H */

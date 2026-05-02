#ifndef XF_JSON_ENC_BUF_H
#define XF_JSON_ENC_BUF_H

#include <string.h>

/*
 * Buffer-append helper used by encode_*.c functions that build JSON manually.
 * Requires local variables in scope: char *buf, size_t pos, size_t cap.
 * Returns -1 from the calling function on overflow.
 */
#define XF_ENC_APPEND(s, len) do { \
    size_t _l = (size_t)(len); \
    if (pos + _l + 1 > cap) return -1; \
    memcpy(buf + pos, (s), _l); pos += _l; buf[pos] = '\0'; \
} while (0)

#endif /* XF_JSON_ENC_BUF_H */

#ifndef XF_B64_H
#define XF_B64_H

#include <stddef.h>
#include <stdint.h>

/* Minimum output buffer size for xf_b64_encode (includes NUL terminator). */
#define XF_B64_ENCODE_LEN(n) (((n) + 2u) / 3u * 4u + 1u)

/*
 * Encodes in[0..in_len) as Base64 into out[0..cap).
 * Returns the number of characters written (not counting the NUL), or -1 if
 * out is NULL or cap < XF_B64_ENCODE_LEN(in_len).
 */
int xf_b64_encode(const uint8_t *in, size_t in_len, char *out, size_t cap);

#endif

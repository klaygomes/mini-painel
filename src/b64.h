#ifndef XF_B64_H
#define XF_B64_H

#include <stddef.h>
#include <stdint.h>
#include "buf.h"

/* Minimum output buffer size for xf_b64_encode (includes NUL terminator). */
#define XF_B64_ENCODE_LEN(n) (((n) + 2u) / 3u * 4u + 1u)

/*
 * Encodes in.ptr[0..in.len) as Base64 into out.ptr[0..out.cap).
 * Returns the number of characters written (not counting the NUL), or -1 if
 * out.ptr is NULL or out.cap < XF_B64_ENCODE_LEN(in.len).
 */
int xf_b64_encode(xf_byte_slice_t in, xf_out_buf_t out);

#endif

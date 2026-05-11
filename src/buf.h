#ifndef XF_BUF_H
#define XF_BUF_H

#include <stddef.h>
#include <stdint.h>

/* Read-only string slice: pointer + byte count (int, matching mjson lengths). */
typedef struct { const char    *ptr; int    len; } xf_str_slice_t;

/* Mutable char output buffer: pointer + capacity. */
typedef struct { char          *ptr; size_t cap; } xf_out_buf_t;

/* Read-only byte slice: pointer + length. */
typedef struct { const uint8_t *ptr; size_t len; } xf_byte_slice_t;

/* Mutable byte buffer: pointer + length. */
typedef struct { uint8_t       *ptr; size_t len; } xf_byte_buf_t;

#endif /* XF_BUF_H */

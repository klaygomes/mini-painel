#ifndef XF_BUF_H
#define XF_BUF_H

#include <stddef.h>
#include <stdint.h>

/* Read-only string slice: pointer + byte count (int, matching mjson lengths). */
typedef struct { const char    *ptr; int    len; } xf_str_slice_t;

/* Mutable char output buffer: pointer + capacity. */
typedef struct { char          *ptr; size_t cap; } xf_str_buf_t;

/* Read-only byte slice: pointer + length. */
typedef struct { const uint8_t *ptr; size_t len; } xf_byte_slice_t;

/* Mutable byte buffer: pointer + length. */
typedef struct { uint8_t       *ptr; size_t len; } xf_byte_buf_t;

/* clang-format off */
#define XF_STR_SLICE(p, n)  ((xf_str_slice_t) { (const char *)(p),    (int)(n)    })
#define XF_STR_BUF(p, c)    ((xf_str_buf_t)   { (char *)(p),          (size_t)(c) })
#define XF_BYTE_SLICE(p, n) ((xf_byte_slice_t){ (const uint8_t *)(p), (size_t)(n) })
#define XF_BYTE_BUF(p, n)   ((xf_byte_buf_t)  { (uint8_t *)(p),       (size_t)(n) })
/* clang-format on */

#endif /* XF_BUF_H */

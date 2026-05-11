#ifndef XF_JSON_DECODE_UTIL_H
#define XF_JSON_DECODE_UTIL_H

#include <stddef.h>
#include "theme.h"          /* xf_rgba_t */
#include "json_str_slice.h" /* xf_str_slice_t */

/*
 * All helpers return 0 on success or -1 on type mismatch.
 * If the path is absent the destination is left unchanged and 0 is returned.
 * In patch mode (patch != 0) absent paths are silently skipped.
 * In non-patch mode absent paths also succeed because the caller zeroed
 * the data block before decoding.
 */

int xf_decode_str  (xf_str_slice_t s, const char *path,
                    xf_str_buf_t dst, int patch);

/* "#RGB", "#RRGGBB", "#RRGGBBAA" — anything else returns -1. */
int xf_decode_color(xf_str_slice_t s, const char *path,
                    xf_rgba_t *out, int patch);

int xf_decode_float(xf_str_slice_t s, const char *path,
                    float *out, int patch);

int xf_decode_int  (xf_str_slice_t s, const char *path,
                    int *out, int patch);

int xf_decode_bool (xf_str_slice_t s, const char *path,
                    int *out, int patch);

int xf_decode_float_array(xf_str_slice_t s, const char *path,
                          float *dst, int max_count, int *out_count,
                          int patch);

typedef int (*xf_elem_decode_fn)(xf_str_slice_t s, void *elem, int patch);

int xf_decode_object_array(xf_str_slice_t s, const char *path,
                           void *array_base, size_t stride, int max_count,
                           int *out_count, xf_elem_decode_fn fn, int patch);

#endif

#ifndef XF_JSON_ENCODE_H
#define XF_JSON_ENCODE_H

#include <stddef.h>
#include "buf.h"

typedef struct {
    char  *buf;
    size_t cap;
    size_t len;
    int    truncated;
    int    any_error;
    int    first_result;   /* used to decide whether to emit a comma */
    size_t ok_offset;      /* byte offset of the "true"/"false" bool in buf */
} xf_encoder_t;

void xf_encoder_init         (xf_encoder_t *e, xf_out_buf_t out);
void xf_encoder_begin        (xf_encoder_t *e);
void xf_encoder_result       (xf_encoder_t *e, int idx, const char *op, const char *id);
void xf_encoder_render_result(xf_encoder_t *e, int idx, int page, int page_count,
                              int x, int y, int w, int h);
void xf_encoder_error        (xf_encoder_t *e, int idx, const char *op, const char *id,
                              const char *msg);
void xf_encoder_list_begin   (xf_encoder_t *e, int idx, int count, int page_count);
void xf_encoder_list_end     (xf_encoder_t *e);
void xf_encoder_raw          (xf_encoder_t *e, xf_str_slice_t s);
void xf_encoder_end          (xf_encoder_t *e, int ok);

#endif

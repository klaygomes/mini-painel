#ifndef XF_JSON_API_H
#define XF_JSON_API_H

#include <stddef.h>
#include <stdint.h>

#define XF_JSON_CANVAS_SIZE(w, h) ((size_t)(w) * (size_t)(h) * 3u)

typedef struct xf_json_ctx xf_json_ctx_t;

typedef struct {
    const void *buf;
    size_t      size;
} xf_json_buf_t;

typedef struct {
    xf_json_ctx_t *ctx;
    xf_json_buf_t  json;
    xf_json_buf_t  canvas;
    xf_json_buf_t  out_json;
} xf_json_exec_req_t;

xf_json_ctx_t *xf_json_create (int width, int height, int padding);
void           xf_json_destroy(xf_json_ctx_t *ctx);

int xf_json_exec(const xf_json_exec_req_t *req);

int xf_json_page_count(const xf_json_ctx_t *ctx);

#endif

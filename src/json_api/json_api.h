#ifndef XF_JSON_API_H
#define XF_JSON_API_H

#include <stddef.h>
#include <stdint.h>

#define XF_JSON_CANVAS_SIZE(w, h) ((size_t)(w) * (size_t)(h) * 3u)

typedef struct xf_json_ctx xf_json_ctx_t;

xf_json_ctx_t *xf_json_create (int width, int height, int padding);
void           xf_json_destroy(xf_json_ctx_t *ctx);

int xf_json_exec(xf_json_ctx_t *ctx,
                 const char *json, size_t json_len,
                 uint8_t *canvas,  size_t canvas_cap,
                 char    *out_json, size_t out_json_cap);

int xf_json_page_count(const xf_json_ctx_t *ctx);

#endif

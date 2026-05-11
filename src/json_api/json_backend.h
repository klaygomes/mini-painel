#ifndef XF_JSON_BACKEND_H
#define XF_JSON_BACKEND_H

#include <stddef.h>
#include <stdint.h>

#include "json_kind.h"
#include "buf.h" /* xf_str_slice_t, xf_byte_buf_t */

typedef struct xf_json_backend xf_json_backend_t;

typedef struct {
    const char   *id;
    const char   *kind;
    int           page;
    int           index;
    int           dirty;
    const void   *data;
    xf_encode_fn  encode;
} xf_json_backend_row_view_t;

xf_json_backend_t *xf_json_backend_dashboard_create(int width, int height, int padding);
void               xf_json_backend_destroy(xf_json_backend_t *backend);

int xf_json_backend_page_count(const xf_json_backend_t *backend);

int xf_json_backend_row_add(xf_json_backend_t *backend,
                            const char *id,
                            xf_str_slice_t data_json,
                            const char **err_msg);

int xf_json_backend_row_update(xf_json_backend_t *backend,
                               const char *id,
                               xf_str_slice_t data_json,
                               const char **err_msg);

int xf_json_backend_row_remove(xf_json_backend_t *backend,
                               const char *id,
                               const char **err_msg);

int xf_json_backend_row_move(xf_json_backend_t *backend,
                             const char *id,
                             int dir,
                             const char **err_msg);

int xf_json_backend_page_render(xf_json_backend_t *backend,
                                int page,
                                xf_byte_buf_t canvas,
                                int *page_count,
                                int *dx,
                                int *dy,
                                int *dw,
                                int *dh,
                                const char **err_msg);

int xf_json_backend_row_count(const xf_json_backend_t *backend);

int xf_json_backend_row_view(const xf_json_backend_t *backend,
                             int idx,
                             xf_json_backend_row_view_t *view);

#endif

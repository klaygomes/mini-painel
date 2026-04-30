#ifndef XF_GFX_STATUS_ICON_H
#define XF_GFX_STATUS_ICON_H

#include "gfx.h"

typedef enum {
    XF_STATUS_INPUT   = 0,
    XF_STATUS_RUNNING,
    XF_STATUS_IDLE,
    XF_STATUS_DONE
} xf_status_t;

void xf_gfx_status_icon(xf_draw_ctx_t *ctx, double cx, double cy, double r,
                          xf_status_t status);

#endif /* XF_GFX_STATUS_ICON_H */

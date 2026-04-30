#include "gfx/status_icon.h"
#include "gfx/dot.h"

void xf_gfx_status_icon(xf_draw_ctx_t *ctx, double cx, double cy, double r,
                          xf_status_t status)
{
    const xf_theme_t *t = xf_gfx_get_theme();
    xf_rgba_t color;

    switch (status) {
        case XF_STATUS_RUNNING: color = t->info;    break;
        case XF_STATUS_IDLE:    color = t->warning; break;
        case XF_STATUS_DONE:    color = t->success; break;
        default:                color = t->offline; break;
    }

    xf_gfx_dot(ctx, cx, cy, r, color);
}

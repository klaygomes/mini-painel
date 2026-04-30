#include "gfx/online_dot.h"

void xf_gfx_online_dot(xf_draw_ctx_t *ctx, double cx, double cy, xf_rgba_t color)
{
    const xf_theme_t *t = xf_gfx_get_theme();
    xf_draw_circle(ctx, cx, cy, 4.0, t->white);
    xf_draw_circle(ctx, cx, cy, 2.8, color);
}

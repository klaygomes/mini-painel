#include "gfx/dot.h"

void xf_gfx_dot(xf_draw_ctx_t *ctx, double cx, double cy, double r, xf_rgba_t color)
{
    xf_draw_circle(ctx, cx, cy, r, color);
}

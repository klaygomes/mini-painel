#include "gfx/icon_check.h"

void xf_gfx_icon_check(xf_draw_ctx_t *ctx, double cx, double cy, double r,
                         xf_rgba_t color)
{
    const xf_theme_t *t = xf_gfx_get_theme();
    xf_draw_circle(ctx, cx, cy, r, color);
    xf_draw_begin_path(ctx);
    xf_draw_move_to(ctx, cx - r * 0.35, cy);
    xf_draw_line_to(ctx, cx - r * 0.05, cy + r * 0.35);
    xf_draw_line_to(ctx, cx + r * 0.40, cy - r * 0.30);
    xf_draw_stroke(ctx, t->on_color, r * 0.18, XF_LINE_CAP_ROUND);
}

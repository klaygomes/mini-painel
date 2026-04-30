#include "gfx/checkbox.h"

void xf_gfx_checkbox(xf_draw_ctx_t *ctx, double x, double cy, int checked)
{
    const xf_theme_t *t = xf_gfx_get_theme();
    double box_y = cy - 5.5;

    if (checked) {
        xf_draw_fill_round_rect(ctx, x, box_y, 11.0, 11.0, 2.0, t->success);
        xf_draw_begin_path(ctx);
        xf_draw_move_to(ctx, x + 1.5,  box_y + 11.0 * 0.55);
        xf_draw_line_to(ctx, x + 11.0 * 0.42, box_y + 11.0 - 2.0);
        xf_draw_line_to(ctx, x + 11.0 - 1.5,  box_y + 2.0);
        xf_draw_stroke(ctx, t->white, 1.5, XF_LINE_CAP_ROUND);
    } else {
        xf_draw_stroke_round_rect(ctx, x, box_y, 11.0, 11.0, 2.0, t->surface_border, 1.0);
    }
}

#include "gfx/strikethrough.h"

void xf_gfx_strikethrough(xf_draw_ctx_t *ctx,
                            double x, double baseline,
                            double width, double font_size)
{
    const xf_theme_t *t = xf_gfx_get_theme();
    double sy = baseline - font_size * 0.35;
    xf_draw_begin_path(ctx);
    xf_draw_move_to(ctx, x,         sy);
    xf_draw_line_to(ctx, x + width, sy);
    xf_draw_stroke(ctx, t->text_faint, 1.0, XF_LINE_CAP_BUTT);
}

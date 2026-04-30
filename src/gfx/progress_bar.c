#include "gfx/progress_bar.h"

void xf_gfx_progress_bar(xf_draw_ctx_t *ctx,
                           double x, double y, double w, double h,
                           double fraction, xf_rgba_t fill)
{
    const xf_theme_t *t = xf_gfx_get_theme();
    xf_draw_fill_round_rect(ctx, x, y, w, h, 3.0, t->surface_card);
    double f  = fraction < 0.0 ? 0.0 : fraction > 1.0 ? 1.0 : fraction;
    double fw = w * f;
    if (fw > 0.0)
        xf_draw_fill_round_rect(ctx, x, y, fw, h, 3.0, fill);
}

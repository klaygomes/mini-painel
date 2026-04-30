#include "gfx/pill.h"

double xf_gfx_pill_width(xf_draw_ctx_t *ctx, const char *text, int weight)
{
    return xf_draw_measure_text(ctx, text, FONT_SM, weight) + 10.0;
}

void xf_gfx_pill(xf_draw_ctx_t *ctx, double cx, double cy,
                  const char *text, int weight, xf_rgba_t bg, xf_rgba_t fg)
{
    double pw = xf_gfx_pill_width(ctx, text, weight);
    xf_draw_fill_round_rect(ctx, cx - pw / 2.0, cy - 7.0, pw, 14.0, 7.0, bg);
    xf_draw_text(ctx, text, cx, cy + 2.5, &(xf_text_opts_t){
        .size = FONT_SM, .weight = weight, .color = fg, .align = XF_TEXT_CENTER
    });
}

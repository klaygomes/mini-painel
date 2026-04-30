#include "gfx/chip.h"

double xf_gfx_chip_width(xf_draw_ctx_t *ctx, const char *text)
{
    return xf_draw_measure_text(ctx, text, FONT_MD, WEIGHT_MEDIUM) + 10.0;
}

void xf_gfx_chip(xf_draw_ctx_t *ctx, double x, double cy,
                  const char *text, xf_rgba_t bg, xf_rgba_t fg)
{
    double cw = xf_gfx_chip_width(ctx, text);
    xf_draw_fill_round_rect(ctx, x, cy - 8.0, cw, 16.0, 8.0, bg);
    xf_draw_text(ctx, text, x + 5.0, cy + 5.0, &(xf_text_opts_t){
        .size = FONT_MD, .weight = WEIGHT_MEDIUM, .color = fg, .family = xf_gfx_get_theme()->font_mono
    });
}

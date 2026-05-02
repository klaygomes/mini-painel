#include "gfx/avatar.h"

void xf_gfx_avatar(xf_draw_ctx_t *ctx, double cx, double cy, double r,
                    xf_rgba_t color, const char *initials)
{
    const xf_theme_t *t = xf_gfx_get_theme();
    xf_draw_circle(ctx, cx, cy, r, color);

    double font_size, offset;
    if (r < 10.0) {
        font_size = FONT_XS;
        offset    = 4.5;
    } else if (r <= 12.0) {
        font_size = FONT_MD;
        offset    = 4.0;
    } else {
        font_size = FONT_SM;
        offset    = 5.0;
    }

    xf_draw_text(ctx, initials, cx, cy + offset, &(xf_text_opts_t){
        .size = font_size, .weight = WEIGHT_BOLD, .color = t->on_color, .align = XF_TEXT_CENTER
    });
}

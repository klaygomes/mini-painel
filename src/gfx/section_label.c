#include "gfx/section_label.h"

void xf_gfx_section_label(xf_draw_ctx_t *ctx, const char *text, double y)
{
    const xf_theme_t *t = xf_gfx_get_theme();
    xf_draw_text(ctx, text, LAY_PAD_X, y, &(xf_text_opts_t){
        .size = FONT_MD, .weight = WEIGHT_SEMIBOLD, .color = t->text_muted
    });
}

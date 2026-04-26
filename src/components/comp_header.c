#include "comp_header.h"
#include "comp_gfx.h"

static void draw(xf_draw_ctx_t *ctx, void *user_data)
{
    const xf_theme_t        *t = xf_get_theme();
    const comp_header_data_t *d = user_data;

    double baseline = 13.0;

    xf_draw_text(ctx, d->date, LAY_PAD_X, baseline, &(xf_text_opts_t){
        .size = FONT_LG, .weight = WEIGHT_NORMAL, .color = t->text_muted
    });

    double tw    = xf_draw_measure_text(ctx, d->status_text, FONT_LG, 400);
    double right = xf_draw_width(ctx) - LAY_PAD_X;

    xf_draw_text(ctx, d->status_text, right, baseline, &(xf_text_opts_t){
        .size = FONT_LG, .weight = WEIGHT_NORMAL, .color = t->text_muted, .align = XF_TEXT_RIGHT
    });

    xf_gfx_dot(ctx, right - tw - 8.0, baseline - 3.0, 3.0, d->status_dot);
}

static void render(xf_component_t *self, uint8_t *buf, int w, int h)
{
    xf_render(buf, w, h, draw, self->ctx);
}

xf_component_t comp_header_create(comp_header_data_t *data)
{
    xf_component_t c = XF_COMPONENT_DATA(render, data);
    return c;
}

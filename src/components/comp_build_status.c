#include "comp_build_status.h"
#include "comp_gfx.h"

static void draw(xf_draw_ctx_t *ctx, void *user_data)
{
    const xf_theme_t              *t = xf_get_theme();
    const comp_build_status_data_t *d = user_data;

    xf_draw_text(ctx, d->branch, LAY_PAD_X, 15.0, &(xf_text_opts_t){
        .size = FONT_LG, .weight = WEIGHT_SEMIBOLD, .color = t->text_primary
    });

    xf_draw_text(ctx, d->build_id, (double)xf_draw_width(ctx) - LAY_PAD_X, 15.0, &(xf_text_opts_t){
        .size = FONT_MD, .weight = WEIGHT_NORMAL, .color = t->text_muted, .align = XF_TEXT_RIGHT
    });

    xf_draw_text(ctx, d->duration, LAY_PAD_X, 31.0, &(xf_text_opts_t){
        .size = FONT_MD, .weight = WEIGHT_NORMAL, .color = t->text_faint
    });

    double pill_cx = (double)xf_draw_width(ctx) - LAY_PAD_X
                     - xf_gfx_pill_width(ctx, d->status, WEIGHT_SEMIBOLD) / 2.0;
    xf_gfx_pill(ctx, pill_cx, 28.0, d->status, WEIGHT_SEMIBOLD, d->status_color, d->status_fg);
}

static void render(xf_component_t *self, uint8_t *buf, int w, int h)
{
    xf_render(buf, w, h, draw, self->ctx);
}

xf_component_t comp_build_status_create(comp_build_status_data_t *data)
{
    xf_component_t c = XF_COMPONENT_DATA(render, data);
    return c;
}

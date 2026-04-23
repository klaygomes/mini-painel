#include "comp_build_status.h"
#include "draw.h"
#include "layout.h"

static void draw(xf_draw_ctx_t *ctx, int w, int h, void *user_data)
{
    const xf_theme_t              *t = xf_get_theme();
    const comp_build_status_data_t *d = user_data;
    (void)h;

    xf_draw_text(ctx, d->branch, LAY_PAD_X, 15.0, &(xf_text_opts_t){
        .size = FONT_LG, .weight = 600, .color = t->text_primary
    });

    xf_draw_text(ctx, d->build_id, (double)w - LAY_PAD_X, 15.0, &(xf_text_opts_t){
        .size = FONT_MD, .weight = 400, .color = t->text_muted, .align = XF_TEXT_RIGHT
    });

    xf_draw_text(ctx, d->duration, LAY_PAD_X, 31.0, &(xf_text_opts_t){
        .size = FONT_MD, .weight = 400, .color = t->text_faint
    });

    double pill_w = xf_draw_measure_text(ctx, d->status, FONT_SM, 600) + 12.0;
    double pill_x = (double)w - LAY_PAD_X - pill_w;
    xf_draw_fill_round_rect(ctx, pill_x, 21.0, pill_w, 14.0, 7.0, d->status_color);
    xf_draw_text(ctx, d->status, pill_x + pill_w / 2.0, 31.0, &(xf_text_opts_t){
        .size = FONT_SM, .weight = 600, .color = d->status_fg, .align = XF_TEXT_CENTER
    });
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

#include "comp_deploy.h"
#include "comp_gfx.h"

static void draw(xf_draw_ctx_t *ctx, void *user_data)
{
    const xf_theme_t        *t = xf_get_theme();
    const comp_deploy_data_t *d = user_data;

    double cy       = (double)xf_draw_height(ctx) / 2.0;
    double baseline = cy + 5.0;

    xf_gfx_dot(ctx, 10.0, cy, 5.0, t->deploy_bar);

    xf_gfx_chip(ctx, 20.0, cy, d->branch, t->deploy_chip_bg, t->deploy_text_dark);
    double chip_end = 20.0 + xf_gfx_chip_width(ctx, d->branch);
    xf_draw_text(ctx, d->time_ago, chip_end + 6.0, baseline, &(xf_text_opts_t){
        .size = FONT_MD, .weight = WEIGHT_NORMAL, .color = t->deploy_text
    });

    xf_draw_text(ctx, d->label, (double)xf_draw_width(ctx) - LAY_PAD_X, baseline, &(xf_text_opts_t){
        .size = FONT_MD, .weight = WEIGHT_NORMAL, .color = t->deploy_text_dark, .align = XF_TEXT_RIGHT
    });
}

static void render(xf_component_t *self, uint8_t *buf, int w, int h)
{
    xf_render(buf, w, h, draw, self->ctx);
}

xf_component_t comp_deploy_create(comp_deploy_data_t *data)
{
    xf_component_t c = XF_COMPONENT_DATA(render, data);
    return c;
}

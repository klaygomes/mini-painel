#include "comp_deploy.h"
#include "draw.h"
#include "layout.h"

static void draw(xf_draw_ctx_t *ctx, int w, int h, void *user_data)
{
    const xf_theme_t        *t = xf_get_theme();
    const comp_deploy_data_t *d = user_data;
    (void)w;

    double cy       = (double)h / 2.0;
    double baseline = cy + 5.0;

    xf_draw_circle(ctx, 10.0, cy, 5.0, t->deploy_bar);

    double branch_w = xf_draw_measure_text(ctx, d->branch, FONT_MD, 500);
    xf_draw_fill_round_rect(ctx, 20.0, cy - 8.0, branch_w + 10.0, 16.0,
                            8.0, t->deploy_chip_bg);
    xf_draw_text(ctx, d->branch, 25.0, baseline, &(xf_text_opts_t){
        .size = FONT_MD, .weight = 500, .color = t->deploy_text_dark
    });

    double chip_end = 20.0 + branch_w + 10.0;
    xf_draw_text(ctx, d->time_ago, chip_end + 6.0, baseline, &(xf_text_opts_t){
        .size = FONT_MD, .weight = 400, .color = t->deploy_text
    });

    xf_draw_text(ctx, d->label, (double)w - LAY_PAD_X, baseline, &(xf_text_opts_t){
        .size = FONT_MD, .weight = 400, .color = t->deploy_text_dark, .align = XF_TEXT_RIGHT
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

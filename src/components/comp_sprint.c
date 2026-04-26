#include "comp_sprint.h"
#include "comp_gfx.h"

#define BAR_H  6

static void draw(xf_draw_ctx_t *ctx, void *user_data)
{
    const xf_theme_t        *t = xf_get_theme();
    const comp_sprint_data_t *d = user_data;

    xf_draw_text(ctx, d->title, LAY_PAD_X, 14.0, &(xf_text_opts_t){
        .size = FONT_LG, .weight = WEIGHT_BOLD, .color = t->text_primary
    });

    xf_draw_text(ctx, d->time_left, (double)xf_draw_width(ctx) - LAY_PAD_X, 14.0, &(xf_text_opts_t){
        .size = FONT_MD, .weight = WEIGHT_NORMAL, .color = t->text_faint, .align = XF_TEXT_RIGHT
    });

    xf_draw_text(ctx, d->progress_label, LAY_PAD_X, 30.0, &(xf_text_opts_t){
        .size = FONT_MD, .weight = WEIGHT_NORMAL, .color = t->text_secondary
    });

    double bar_x = LAY_PAD_X;
    double bar_w = (double)xf_draw_width(ctx) - 2.0 * LAY_PAD_X;
    double bar_y = 36.0;

    xf_gfx_progress_bar(ctx, bar_x, bar_y, bar_w, BAR_H,
                         (double)d->percent, t->purple_bar);
}

static void render(xf_component_t *self, uint8_t *buf, int w, int h)
{
    xf_render(buf, w, h, draw, self->ctx);
}

xf_component_t comp_sprint_create(comp_sprint_data_t *data)
{
    xf_component_t c = XF_COMPONENT_DATA(render, data);
    return c;
}

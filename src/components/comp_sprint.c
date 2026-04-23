#include "comp_sprint.h"
#include "draw.h"

#define PAD_X  8
#define BAR_H  6

static void draw(xf_draw_ctx_t *ctx, int w, int h, void *user_data)
{
    const xf_theme_t        *t = xf_get_theme();
    const comp_sprint_data_t *d = user_data;
    (void)h;

    xf_draw_text(ctx, d->title, PAD_X, 12.0, &(xf_text_opts_t){
        .size = 10, .weight = 700, .color = t->text_primary
    });

    xf_draw_text(ctx, d->time_left, (double)w - PAD_X, 12.0, &(xf_text_opts_t){
        .size = 9, .weight = 400, .color = t->text_faint, .align = XF_TEXT_RIGHT
    });

    xf_draw_text(ctx, d->progress_label, PAD_X, 26.0, &(xf_text_opts_t){
        .size = 9, .weight = 400, .color = t->text_secondary
    });

    double bar_x     = PAD_X;
    double bar_w     = (double)w - 2.0 * PAD_X;
    double bar_y     = 31.0;

    xf_draw_fill_round_rect(ctx, bar_x, bar_y, bar_w, BAR_H, 3.0, t->surface_card);

    double fill = bar_w * (double)d->percent;
    if (fill > bar_w) fill = bar_w;
    if (fill > 0)
        xf_draw_fill_round_rect(ctx, bar_x, bar_y, fill, BAR_H, 3.0, t->purple_bar);
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

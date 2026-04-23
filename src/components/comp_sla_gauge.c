#include "comp_sla_gauge.h"
#include "draw.h"

#define HEADER_H   16
#define ROW_H      20
#define BAR_H       6
#define BAR_MARGIN  8

static void draw(xf_draw_ctx_t *ctx, int w, int h, void *user_data)
{
    const xf_theme_t           *t = xf_get_theme();
    const comp_sla_gauge_data_t *d = user_data;
    (void)h;

    xf_draw_text(ctx, d->title, 8.0, 11.0, &(xf_text_opts_t){
        .size = 9, .weight = 600, .color = t->text_muted
    });

    double bar_x     = (double)BAR_MARGIN;
    double bar_avail = (double)w - 2.0 * BAR_MARGIN;
    double y         = HEADER_H;

    for (int i = 0; i < d->count; i++) {
        const comp_sla_row_t *r = &d->rows[i];

        xf_draw_text(ctx, r->label, bar_x, y + 11.0, &(xf_text_opts_t){
            .size = 9, .weight = 400, .color = t->text_secondary
        });

        xf_draw_text(ctx, r->value, (double)w - (double)BAR_MARGIN, y + 11.0,
                     &(xf_text_opts_t){
                         .size = 9, .weight = 600, .color = t->text_primary,
                         .align = XF_TEXT_RIGHT
                     });

        xf_draw_fill_round_rect(ctx, bar_x, y + 13.0, bar_avail, BAR_H,
                                3.0, t->surface_card);

        /* Fill — clamp to track width to avoid overdraw at 100 % */
        double fill = bar_avail * (double)r->percent / 100.0;
        if (fill > bar_avail) fill = bar_avail;
        if (fill > 0)
            xf_draw_fill_round_rect(ctx, bar_x, y + 13.0, fill, BAR_H,
                                    3.0, r->bar);

        y += ROW_H;
    }
}

static void render(xf_component_t *self, uint8_t *buf, int w, int h)
{
    xf_render(buf, w, h, draw, self->ctx);
}

xf_component_t comp_sla_gauge_create(comp_sla_gauge_data_t *data)
{
    xf_component_t c = XF_COMPONENT_DATA(render, data);
    return c;
}

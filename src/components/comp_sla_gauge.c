#include "comp_sla_gauge.h"
#include "comp_gfx.h"

#define HEADER_H   LAY_HEADER_H
#define ROW_H      LAY_ROW_MD
#define BAR_H       6
#define BAR_MARGIN  8

static void draw(xf_draw_ctx_t *ctx, void *user_data)
{
    const xf_theme_t           *t = xf_get_theme();
    const comp_sla_gauge_data_t *d = user_data;

    xf_gfx_section_label(ctx, d->title, 13.0);

    double bar_x     = (double)BAR_MARGIN;
    double bar_avail = (double)xf_draw_width(ctx) - 2.0 * BAR_MARGIN;
    double y         = HEADER_H;

    for (int i = 0; i < d->count; i++) {
        const comp_sla_row_t *r = &d->rows[i];

        xf_draw_text(ctx, r->label, bar_x, y + 13.0, &(xf_text_opts_t){
            .size = FONT_MD, .weight = WEIGHT_NORMAL, .color = t->text_secondary
        });

        xf_draw_text(ctx, r->value, (double)xf_draw_width(ctx) - (double)BAR_MARGIN, y + 13.0,
                     &(xf_text_opts_t){
                         .size = FONT_MD, .weight = WEIGHT_SEMIBOLD, .color = t->text_primary,
                         .align = XF_TEXT_RIGHT
                     });

        xf_gfx_progress_bar(ctx, bar_x, y + 15.0, bar_avail, BAR_H,
                             (double)r->percent / 100.0, r->bar);

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

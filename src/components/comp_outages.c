/* Each row carries its own colour set so the caller controls severity-to-colour
 * mapping; this component never encodes those rules internally. */

#include "comp_outages.h"
#include "comp_gfx.h"

#define HEADER_H LAY_HEADER_H
#define ROW_H    LAY_ROW_LG
#define ROW_GAP  LAY_GAP_MD

static void draw(xf_draw_ctx_t *ctx, void *user_data)
{
    const comp_outages_data_t *d = user_data;

    xf_gfx_section_label(ctx, d->title, 13.0);

    double y = HEADER_H;

    for (int i = 0; i < d->count; i++) {
        const comp_outage_row_t *r = &d->rows[i];

        xf_draw_fill_round_rect(ctx, 0, y, (double)xf_draw_width(ctx), ROW_H, 3.0, r->row_bg);

        xf_gfx_dot(ctx, 10.0, y + (double)ROW_H / 2.0, 3.5, r->dot);

        xf_draw_text(ctx, r->service, 20.0, y + 17.0, &(xf_text_opts_t){
            .size = FONT_LG, .weight = WEIGHT_SEMIBOLD, .color = r->title_fg, .max_width = 120.0
        });

        double pill_w = xf_gfx_pill_width(ctx, r->status, WEIGHT_MEDIUM);
        double pill_cx = (double)xf_draw_width(ctx) - 8.0 - pill_w / 2.0;
        xf_gfx_pill(ctx, pill_cx, y + 14.0, r->status, WEIGHT_MEDIUM, r->pill_bg, r->pill_fg);

        xf_draw_text(ctx, r->duration, pill_cx - pill_w / 2.0 - 6.0, y + 17.0, &(xf_text_opts_t){
            .size = FONT_MD, .weight = WEIGHT_NORMAL, .color = r->pill_fg, .align = XF_TEXT_RIGHT
        });

        y += ROW_H + ROW_GAP;
    }
}

static void render(xf_component_t *self, uint8_t *buf, int w, int h)
{
    xf_render(buf, w, h, draw, self->ctx);
}

xf_component_t comp_outages_create(comp_outages_data_t *data)
{
    xf_component_t c = XF_COMPONENT_DATA(render, data);
    return c;
}

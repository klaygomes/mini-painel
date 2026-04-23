/* Each row carries its own colour set so the caller controls severity-to-colour
 * mapping; this component never encodes those rules internally. */

#include "comp_outages.h"
#include "draw.h"

#define HEADER_H   16
#define ROW_H      24
#define ROW_GAP     4

static void draw(xf_draw_ctx_t *ctx, int w, int h, void *user_data)
{
    const xf_theme_t         *t = xf_get_theme();
    const comp_outages_data_t *d = user_data;
    (void)h;

    xf_draw_text(ctx, d->title, 8.0, 11.0, &(xf_text_opts_t){
        .size = 9, .weight = 600, .color = t->text_muted
    });

    double y = HEADER_H;

    for (int i = 0; i < d->count; i++) {
        const comp_outage_row_t *r = &d->rows[i];

        xf_draw_fill_round_rect(ctx, 0, y, (double)w, ROW_H, 3.0, r->row_bg);

        xf_draw_circle(ctx, 10.0, y + (double)ROW_H / 2.0, 3.5, r->dot);

        xf_draw_text(ctx, r->service, 20.0, y + 15.0, &(xf_text_opts_t){
            .size = 10, .weight = 600, .color = r->title_fg, .max_width = 120.0
        });

        double pill_w = xf_draw_measure_text(ctx, r->status, 8, 500) + 10.0;
        double pill_x = (double)w - 8.0 - pill_w;
        xf_draw_fill_round_rect(ctx, pill_x, y + 6.0, pill_w, 12.0, 6.0, r->pill_bg);
        xf_draw_text(ctx, r->status, pill_x + pill_w / 2.0, y + 14.5, &(xf_text_opts_t){
            .size = 8, .weight = 500, .color = r->pill_fg, .align = XF_TEXT_CENTER
        });

        xf_draw_text(ctx, r->duration, pill_x - 6.0, y + 15.0, &(xf_text_opts_t){
            .size = 9, .weight = 400, .color = r->pill_fg, .align = XF_TEXT_RIGHT
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

#include "comp_alerts.h"
#include "comp_gfx.h"

#define HEADER_H LAY_HEADER_H
#define ROW_H     LAY_ROW_ALERT
#define ROW_GAP   LAY_GAP_SM

static void draw(xf_draw_ctx_t *ctx, void *user_data)
{
    const xf_theme_t        *t = xf_get_theme();
    const comp_alerts_data_t *d = user_data;

    xf_gfx_section_label(ctx, d->title, 13.0);

    double y = HEADER_H;

    for (int i = 0; i < d->count; i++) {
        const comp_alert_row_t *r = &d->rows[i];

        xf_draw_fill_round_rect(ctx, 0, y, (double)xf_draw_width(ctx), ROW_H, 2.0, r->row_bg);

        xf_gfx_dot(ctx, 9.0, y + (double)ROW_H / 2.0, 3.0, r->dot);

        xf_draw_text(ctx, r->message, 18.0, y + 13.0, &(xf_text_opts_t){
            .size = FONT_LG, .weight = WEIGHT_NORMAL, .color = t->text_secondary,
            .max_width = (double)xf_draw_width(ctx) - 70.0
        });

        xf_draw_text(ctx, r->time, (double)xf_draw_width(ctx) - LAY_PAD_X, y + 13.0, &(xf_text_opts_t){
            .size = FONT_SM, .weight = WEIGHT_NORMAL, .color = t->text_faint, .align = XF_TEXT_RIGHT
        });

        y += ROW_H + ROW_GAP;
    }
}

static void render(xf_component_t *self, uint8_t *buf, int w, int h)
{
    xf_render(buf, w, h, draw, self->ctx);
}

xf_component_t comp_alerts_create(comp_alerts_data_t *data)
{
    xf_component_t c = XF_COMPONENT_DATA(render, data);
    return c;
}

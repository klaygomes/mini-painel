/**
 * @file comp_alerts.c
 * @brief Section header + list of alert rows with severity dot and age label.
 */

#include "comp_alerts.h"
#include "draw.h"

#define HEADER_H 16
#define ROW_H     18
#define ROW_GAP    2

static void draw(xf_draw_ctx_t *ctx, int w, int h, void *user_data)
{
    const xf_theme_t        *t = xf_get_theme();
    const comp_alerts_data_t *d = user_data;
    (void)h;

    xf_draw_text(ctx, d->title, 8.0, 11.0, &(xf_text_opts_t){
        .size = 9, .weight = 600, .color = t->text_muted
    });

    double y = HEADER_H;

    for (int i = 0; i < d->count; i++) {
        const comp_alert_row_t *r = &d->rows[i];

        xf_draw_fill_round_rect(ctx, 0, y, (double)w, ROW_H, 2.0, r->row_bg);

        /* Severity dot */
        xf_draw_circle(ctx, 9.0, y + (double)ROW_H / 2.0, 3.0, r->dot);

        /* Message text */
        xf_draw_text(ctx, r->message, 18.0, y + 12.0, &(xf_text_opts_t){
            .size = 10, .weight = 400, .color = t->text_secondary,
            .max_width = (double)w - 70.0
        });

        /* Age right-aligned */
        xf_draw_text(ctx, r->time, (double)w - 8.0, y + 12.0, &(xf_text_opts_t){
            .size = 8, .weight = 400, .color = t->text_faint, .align = XF_TEXT_RIGHT
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

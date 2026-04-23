#include "comp_pr_review.h"
#include "draw.h"
#include <stdio.h>

#define HEADER_H 16
#define ROW_H    20
#define ROW_GAP   3

static void draw(xf_draw_ctx_t *ctx, int w, int h, void *user_data)
{
    const xf_theme_t           *t = xf_get_theme();
    const comp_pr_review_data_t *d = user_data;
    (void)h;

    xf_draw_text(ctx, d->title, 8.0, 11.0, &(xf_text_opts_t){
        .size = 9, .weight = 600, .color = t->text_muted
    });

    double y = HEADER_H;

    for (int i = 0; i < d->count; i++) {
        const comp_pr_row_t *r = &d->rows[i];
        double cy = y + (double)ROW_H / 2.0;

        xf_draw_circle(ctx, 14.0, cy, 8.0, r->avatar_color);
        xf_draw_text(ctx, r->initials, 14.0, cy + 3.5, &(xf_text_opts_t){
            .size = 7, .weight = 700, .color = t->white, .align = XF_TEXT_CENTER
        });

        xf_draw_text(ctx, r->title, 27.0, cy + 4.0, &(xf_text_opts_t){
            .size = 10, .weight = 500, .color = t->text_secondary,
            .max_width = (double)w - 80.0
        });

        xf_draw_text(ctx, r->age, (double)w - 8.0, cy + 4.0, &(xf_text_opts_t){
            .size = 9, .weight = 400, .color = t->text_faint, .align = XF_TEXT_RIGHT
        });

        if (r->reviews > 0) {
            char badge[8];
            snprintf(badge, sizeof(badge), "%d", r->reviews);
            double bw = xf_draw_measure_text(ctx, badge, 8, 600) + 8.0;
            double bx = (double)w - 8.0 - bw
                        - xf_draw_measure_text(ctx, r->age, 9, 400) - 6.0;
            xf_draw_fill_round_rect(ctx, bx, cy - 6.0, bw, 12.0, 6.0, t->success_bg);
            xf_draw_text(ctx, badge, bx + bw / 2.0, cy + 4.0, &(xf_text_opts_t){
                .size = 8, .weight = 600, .color = t->success_fg, .align = XF_TEXT_CENTER
            });
        }

        y += ROW_H + ROW_GAP;
    }
}

static void render(xf_component_t *self, uint8_t *buf, int w, int h)
{
    xf_render(buf, w, h, draw, self->ctx);
}

xf_component_t comp_pr_review_create(comp_pr_review_data_t *data)
{
    xf_component_t c = XF_COMPONENT_DATA(render, data);
    return c;
}

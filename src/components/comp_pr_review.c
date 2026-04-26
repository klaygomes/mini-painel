#include "comp_pr_review.h"
#include "comp_gfx.h"
#include <stdio.h>

#define HEADER_H LAY_HEADER_H
#define ROW_H    LAY_ROW_MD
#define ROW_GAP   3

static void draw(xf_draw_ctx_t *ctx, void *user_data)
{
    const xf_theme_t           *t = xf_get_theme();
    const comp_pr_review_data_t *d = user_data;

    xf_gfx_section_label(ctx, d->title, 13.0);

    double y = HEADER_H;

    for (int i = 0; i < d->count; i++) {
        const comp_pr_row_t *r = &d->rows[i];
        double cy = y + (double)ROW_H / 2.0;

        xf_gfx_avatar(ctx, 14.0, cy, 8.0, r->avatar_color, r->initials);

        xf_draw_text(ctx, r->title, 27.0, cy + 5.0, &(xf_text_opts_t){
            .size = FONT_LG, .weight = WEIGHT_MEDIUM, .color = t->text_secondary,
            .max_width = (double)xf_draw_width(ctx) - 80.0
        });

        xf_draw_text(ctx, r->age, (double)xf_draw_width(ctx) - 8.0, cy + 5.0, &(xf_text_opts_t){
            .size = FONT_MD, .weight = WEIGHT_NORMAL, .color = t->text_faint, .align = XF_TEXT_RIGHT
        });

        if (r->reviews > 0) {
            char badge[8];
            snprintf(badge, sizeof(badge), "%d", r->reviews);
            double bw  = xf_gfx_pill_width(ctx, badge, WEIGHT_SEMIBOLD);
            double age_w = xf_draw_measure_text(ctx, r->age, FONT_MD, WEIGHT_NORMAL);
            double bcx = (double)xf_draw_width(ctx) - 8.0 - age_w - 6.0 - bw / 2.0;
            xf_gfx_pill(ctx, bcx, cy, badge, WEIGHT_SEMIBOLD, t->success_bg, t->success_fg);
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

/* Thresholds encode the operational rule: sustained index >= 10 is an incident
 * (danger), 7-9 is degraded (warning), < 7 is nominal (info). */

#include "comp_error_rate.h"
#include "comp_gfx.h"

#define TITLE_H LAY_TITLE_H
#define BAR_GAP  2

static void draw(xf_draw_ctx_t *ctx, void *user_data)
{
    const xf_theme_t             *t = xf_get_theme();
    const comp_error_rate_data_t  *d = user_data;

    xf_gfx_section_label(ctx, d->title, 12.0);

    xf_draw_text(ctx, d->value, xf_draw_width(ctx) - LAY_PAD_X, 12.0, &(xf_text_opts_t){
        .size = FONT_MD, .weight = WEIGHT_BOLD, .color = t->text_primary, .align = XF_TEXT_RIGHT
    });

    if (d->count <= 0)
        return;

    double chart_x = LAY_PAD_X;
    double chart_y = TITLE_H + 2.0;
    double chart_w = xf_draw_width(ctx) - 2.0 * LAY_PAD_X;
    double chart_h = xf_draw_height(ctx) - chart_y - 4.0;
    double bar_w   = (chart_w - (double)(d->count - 1) * BAR_GAP) / (double)d->count;

    for (int i = 0; i < d->count; i++) {
        xf_rgba_t color = (i >= 10) ? t->danger
                        : (i >=  7) ? t->warning
                                    : t->info;

        double bh = (double)d->bars[i] * chart_h;
        double bx = chart_x + (double)i * (bar_w + BAR_GAP);
        double by = chart_y + chart_h - bh;

        if (bh > 0)
            xf_draw_fill_round_rect(ctx, bx, by, bar_w, bh, 2.0, color);
    }
}

static void render(xf_component_t *self, uint8_t *buf, int w, int h)
{
    xf_render(buf, w, h, draw, self->ctx);
}

xf_component_t comp_error_rate_create(comp_error_rate_data_t *data)
{
    xf_component_t c = XF_COMPONENT_DATA(render, data);
    return c;
}

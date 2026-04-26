/* Filled area and line share the same path points so the alpha fill
 * visually anchors the line without a second render pass. */

#include "comp_sparkline.h"
#include "comp_gfx.h"

#define TITLE_H LAY_TITLE_H

static void draw(xf_draw_ctx_t *ctx, void *user_data)
{
    const xf_theme_t            *t = xf_get_theme();
    const comp_sparkline_data_t  *d = user_data;

    xf_gfx_section_label(ctx, d->title, 12.0);

    xf_draw_text(ctx, d->value, (double)xf_draw_width(ctx) - LAY_PAD_X, 12.0, &(xf_text_opts_t){
        .size = FONT_MD, .weight = WEIGHT_BOLD, .color = t->info_dark, .align = XF_TEXT_RIGHT
    });

    if (d->count < 2)
        return;

    double chart_x = LAY_PAD_X;
    double chart_y = TITLE_H;
    double chart_w = (double)xf_draw_width(ctx) - 2.0 * LAY_PAD_X;
    double chart_h = (double)xf_draw_height(ctx) - TITLE_H - 4.0;
    double step    = chart_w / (double)(d->count - 1);
    double bottom  = chart_y + chart_h;

    xf_draw_begin_path(ctx);
    xf_draw_move_to(ctx, chart_x, bottom);
    for (int i = 0; i < d->count; i++) {
        double px = chart_x + (double)i * step;
        double py = bottom - (double)d->points[i] * chart_h;
        xf_draw_line_to(ctx, px, py);
    }
    xf_draw_line_to(ctx, chart_x + (double)(d->count - 1) * step, bottom);
    xf_draw_close_path(ctx);
    xf_draw_fill(ctx, t->info_fill);

    xf_draw_begin_path(ctx);
    xf_draw_move_to(ctx, chart_x,
                    bottom - (double)d->points[0] * chart_h);
    for (int i = 1; i < d->count; i++) {
        double px = chart_x + (double)i * step;
        double py = bottom - (double)d->points[i] * chart_h;
        xf_draw_line_to(ctx, px, py);
    }
    xf_draw_stroke(ctx, t->info, 1.25, XF_LINE_CAP_ROUND);

    double last_x = chart_x + (double)(d->count - 1) * step;
    double last_y = bottom - (double)d->points[d->count - 1] * chart_h;
    xf_draw_circle(ctx, last_x, last_y, 2.5, t->info);
}

static void render(xf_component_t *self, uint8_t *buf, int w, int h)
{
    xf_render(buf, w, h, draw, self->ctx);
}

xf_component_t comp_sparkline_create(comp_sparkline_data_t *data)
{
    xf_component_t c = XF_COMPONENT_DATA(render, data);
    return c;
}

/**
 * @file comp_build_status.c
 * @brief Two-line build status: branch + build ID on top, duration + outcome pill below.
 */

#include "comp_build_status.h"
#include "draw.h"

#define PAD_X 8

static void draw(xf_draw_ctx_t *ctx, int w, int h, void *user_data)
{
    const xf_theme_t              *t = xf_get_theme();
    const comp_build_status_data_t *d = user_data;
    (void)h;

    /* Branch name on the left */
    xf_draw_text(ctx, d->branch, PAD_X, 13.0, &(xf_text_opts_t){
        .size = 10, .weight = 600, .color = t->text_primary
    });

    /* Build ID right-aligned on the same line */
    xf_draw_text(ctx, d->build_id, (double)w - PAD_X, 13.0, &(xf_text_opts_t){
        .size = 9, .weight = 400, .color = t->text_muted, .align = XF_TEXT_RIGHT
    });

    /* Duration on the left, second row */
    xf_draw_text(ctx, d->duration, PAD_X, 27.0, &(xf_text_opts_t){
        .size = 9, .weight = 400, .color = t->text_faint
    });

    /* Status pill right-aligned, second row */
    double pill_w = xf_draw_measure_text(ctx, d->status, 8, 600) + 12.0;
    double pill_x = (double)w - PAD_X - pill_w;
    xf_draw_fill_round_rect(ctx, pill_x, 19.0, pill_w, 12.0, 6.0, d->status_color);
    xf_draw_text(ctx, d->status, pill_x + pill_w / 2.0, 28.0, &(xf_text_opts_t){
        .size = 8, .weight = 600, .color = d->status_fg, .align = XF_TEXT_CENTER
    });
}

static void render(xf_component_t *self, uint8_t *buf, int w, int h)
{
    xf_render(buf, w, h, draw, self->ctx);
}

xf_component_t comp_build_status_create(comp_build_status_data_t *data)
{
    xf_component_t c = XF_COMPONENT_DATA(render, data);
    return c;
}

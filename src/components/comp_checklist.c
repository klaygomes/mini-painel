/* Strikethrough is measured with xf_draw_measure_text so it always matches
 * the actual rendered text width. */

#include "comp_checklist.h"
#include "draw.h"
#include "layout.h"

#define HEADER_H  LAY_HEADER_H
#define ROW_H     LAY_ROW_SM
#define BOX_SIZE  11.0
#define TEXT_X    24.0
#define FONT_SIZE ((double)FONT_LG)

static void draw(xf_draw_ctx_t *ctx, int w, int h, void *user_data)
{
    const xf_theme_t          *t = xf_get_theme();
    const comp_checklist_data_t *d = user_data;
    (void)h;

    xf_draw_text(ctx, d->title, LAY_PAD_X, 13.0, &(xf_text_opts_t){
        .size = FONT_MD, .weight = 600, .color = t->text_muted
    });

    double y = HEADER_H;

    for (int i = 0; i < d->count; i++) {
        const comp_checklist_item_t *item = &d->items[i];
        double box_y   = y + ((double)ROW_H - BOX_SIZE) / 2.0;
        double baseline = y + 12.0;

        if (item->done) {
            xf_draw_fill_round_rect(ctx, LAY_PAD_X, box_y, BOX_SIZE, BOX_SIZE,
                                    2.0, t->success);

            /* White checkmark: a short diagonal from lower-left to mid,
               then up-right to the top corner */
            xf_draw_begin_path(ctx);
            xf_draw_move_to(ctx,  LAY_PAD_X + 1.5, box_y + BOX_SIZE * 0.55);
            xf_draw_line_to(ctx,  LAY_PAD_X + BOX_SIZE * 0.42, box_y + BOX_SIZE - 2.0);
            xf_draw_line_to(ctx,  LAY_PAD_X + BOX_SIZE - 1.5,  box_y + 2.0);
            xf_draw_stroke(ctx, t->white, 1.5, XF_LINE_CAP_ROUND);

            double tw = xf_draw_measure_text(ctx, item->item, FONT_SIZE, 400);
            xf_draw_text(ctx, item->item, TEXT_X, baseline, &(xf_text_opts_t){
                .size = FONT_SIZE, .weight = 400, .color = t->text_faint,
                .max_width = (double)w - TEXT_X - LAY_PAD_X
            });

            double strike_y = baseline - FONT_SIZE * 0.35;
            double max_tw   = (double)w - TEXT_X - LAY_PAD_X;
            if (tw > max_tw) tw = max_tw;
            xf_draw_begin_path(ctx);
            xf_draw_move_to(ctx, TEXT_X,        strike_y);
            xf_draw_line_to(ctx, TEXT_X + tw,   strike_y);
            xf_draw_stroke(ctx, t->text_faint, 1.0, XF_LINE_CAP_BUTT);
        } else {
            xf_draw_stroke_round_rect(ctx, LAY_PAD_X, box_y, BOX_SIZE, BOX_SIZE,
                                      2.0, t->surface_border, 1.0);

            xf_draw_text(ctx, item->item, TEXT_X, baseline, &(xf_text_opts_t){
                .size = FONT_SIZE, .weight = 400, .color = t->text_secondary,
                .max_width = (double)w - TEXT_X - LAY_PAD_X
            });
        }

        y += ROW_H;
    }
}

static void render(xf_component_t *self, uint8_t *buf, int w, int h)
{
    xf_render(buf, w, h, draw, self->ctx);
}

xf_component_t comp_checklist_create(comp_checklist_data_t *data)
{
    xf_component_t c = XF_COMPONENT_DATA(render, data);
    return c;
}

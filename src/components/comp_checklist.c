/* Strikethrough is measured with xf_draw_measure_text so it always matches
 * the actual rendered text width. */

#include "comp_checklist.h"
#include "comp_gfx.h"

#define HEADER_H  LAY_HEADER_H
#define ROW_H     LAY_ROW_SM
#define TEXT_X    24.0
#define FONT_SIZE ((double)FONT_LG)

static void draw(xf_draw_ctx_t *ctx, void *user_data)
{
    const xf_theme_t          *t = xf_get_theme();
    const comp_checklist_data_t *d = user_data;

    xf_gfx_section_label(ctx, d->title, 13.0);

    double y = HEADER_H;

    for (int i = 0; i < d->count; i++) {
        const comp_checklist_item_t *item = &d->items[i];
        double baseline = y + 12.0;

        if (item->done) {
            xf_gfx_checkbox(ctx, LAY_PAD_X, y + (double)ROW_H / 2.0, 1);

            double tw = xf_draw_measure_text(ctx, item->item, FONT_SIZE, WEIGHT_NORMAL);
            xf_draw_text(ctx, item->item, TEXT_X, baseline, &(xf_text_opts_t){
                .size = FONT_SIZE, .weight = WEIGHT_NORMAL, .color = t->text_faint,
                .max_width = (double)xf_draw_width(ctx) - TEXT_X - LAY_PAD_X
            });

            double max_tw = (double)xf_draw_width(ctx) - TEXT_X - LAY_PAD_X;
            xf_gfx_strikethrough(ctx, TEXT_X, baseline,
                                 tw > max_tw ? max_tw : tw, FONT_SIZE);
        } else {
            xf_gfx_checkbox(ctx, LAY_PAD_X, y + (double)ROW_H / 2.0, 0);

            xf_draw_text(ctx, item->item, TEXT_X, baseline, &(xf_text_opts_t){
                .size = FONT_SIZE, .weight = WEIGHT_NORMAL, .color = t->text_secondary,
                .max_width = (double)xf_draw_width(ctx) - TEXT_X - LAY_PAD_X
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

#include "comp_schedule.h"
#include "draw.h"

#define HEADER_H 16
#define ROW_H    17

static void draw(xf_draw_ctx_t *ctx, int w, int h, void *user_data)
{
    const xf_theme_t          *t = xf_get_theme();
    const comp_schedule_data_t *d = user_data;
    (void)h;

    xf_draw_text(ctx, d->title, 8.0, 11.0, &(xf_text_opts_t){
        .size = 9, .weight = 600, .color = t->text_muted
    });

    double y = HEADER_H;

    for (int i = 0; i < d->count; i++) {
        const comp_schedule_row_t *r = &d->rows[i];

        xf_draw_fill_round_rect(ctx, 4.0, y + 2.0, 3.0, (double)ROW_H - 4.0,
                                1.5, r->bar);

        xf_draw_text(ctx, r->time, 12.0, y + 12.0, &(xf_text_opts_t){
            .size = 9, .weight = 400, .color = t->text_faint, .max_width = 68.0
        });

        xf_draw_text(ctx, r->event, 84.0, y + 12.0, &(xf_text_opts_t){
            .size = 10, .weight = 500, .color = t->text_secondary,
            .max_width = (double)w - 92.0
        });

        y += ROW_H;
    }
}

static void render(xf_component_t *self, uint8_t *buf, int w, int h)
{
    xf_render(buf, w, h, draw, self->ctx);
}

xf_component_t comp_schedule_create(comp_schedule_data_t *data)
{
    xf_component_t c = XF_COMPONENT_DATA(render, data);
    return c;
}

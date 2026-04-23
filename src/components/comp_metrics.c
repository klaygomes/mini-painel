#include "comp_metrics.h"
#include "draw.h"
#include "layout.h"

static void draw(xf_draw_ctx_t *ctx, int w, int h, void *user_data)
{
    const xf_theme_t         *t = xf_get_theme();
    const comp_metrics_data_t *d = user_data;
    (void)h;

    if (d->count <= 0)
        return;

    double card_w = (double)w / (double)d->count;

    for (int i = 0; i < d->count; i++) {
        double x = (double)i * card_w;

        xf_draw_fill_round_rect(ctx, x + 2.0, 2.0, card_w - 4.0, (double)h - 4.0,
                                4.0, t->surface_card);

        xf_draw_text(ctx, d->cards[i].label, x + card_w / 2.0, 16.0,
                     &(xf_text_opts_t){
                         .size = FONT_SM, .weight = 600, .color = t->text_muted,
                         .align = XF_TEXT_CENTER
                     });

        xf_draw_text(ctx, d->cards[i].value, x + card_w / 2.0, 34.0,
                     &(xf_text_opts_t){
                         .size = FONT_HERO, .weight = 700, .color = t->text_primary,
                         .align = XF_TEXT_CENTER
                     });
    }
}

static void render(xf_component_t *self, uint8_t *buf, int w, int h)
{
    xf_render(buf, w, h, draw, self->ctx);
}

xf_component_t comp_metrics_create(comp_metrics_data_t *data)
{
    xf_component_t c = XF_COMPONENT_DATA(render, data);
    return c;
}

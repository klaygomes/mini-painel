#include "comp_oncall.h"
#include "comp_gfx.h"

static void draw(xf_draw_ctx_t *ctx, void *user_data)
{
    const xf_theme_t        *t = xf_get_theme();
    const comp_oncall_data_t *d = user_data;

    double cy = xf_draw_height(ctx) / 2.0;

    xf_gfx_avatar(ctx, 22.0, cy, 12.0, d->avatar_color, d->initials);

    xf_draw_text(ctx, d->name, 42.0, cy - 2.0, &(xf_text_opts_t){
        .size = FONT_XL, .weight = WEIGHT_SEMIBOLD, .color = t->text_secondary, .max_width = 160.0
    });

    xf_draw_text(ctx, d->role, 42.0, cy + 12.0, &(xf_text_opts_t){
        .size = FONT_MD, .weight = WEIGHT_NORMAL, .color = t->text_muted, .max_width = 160.0
    });

    xf_draw_text(ctx, d->phone, xf_draw_width(ctx) - 8.0, cy + 4.0, &(xf_text_opts_t){
        .size = FONT_LG, .weight = WEIGHT_NORMAL, .color = t->text_faint, .align = XF_TEXT_RIGHT
    });
}

static void render(xf_component_t *self, uint8_t *buf, int w, int h)
{
    xf_render(buf, w, h, draw, self->ctx);
}

xf_component_t comp_oncall_create(comp_oncall_data_t *data)
{
    xf_component_t c = XF_COMPONENT_DATA(render, data);
    return c;
}

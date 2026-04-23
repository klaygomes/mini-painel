#include "comp_oncall.h"
#include "draw.h"
#include "layout.h"

static void draw(xf_draw_ctx_t *ctx, int w, int h, void *user_data)
{
    const xf_theme_t        *t = xf_get_theme();
    const comp_oncall_data_t *d = user_data;
    (void)w;

    double cy = (double)h / 2.0;

    xf_draw_circle(ctx, 22.0, cy, 12.0, d->avatar_color);

    xf_draw_text(ctx, d->initials, 22.0, cy + 4.0, &(xf_text_opts_t){
        .size = FONT_MD, .weight = 700, .color = t->white, .align = XF_TEXT_CENTER
    });

    xf_draw_text(ctx, d->name, 42.0, cy - 2.0, &(xf_text_opts_t){
        .size = FONT_XL, .weight = 600, .color = t->text_secondary, .max_width = 160.0
    });

    xf_draw_text(ctx, d->role, 42.0, cy + 12.0, &(xf_text_opts_t){
        .size = FONT_MD, .weight = 400, .color = t->text_muted, .max_width = 160.0
    });

    xf_draw_text(ctx, d->phone, (double)w - 8.0, cy + 4.0, &(xf_text_opts_t){
        .size = FONT_LG, .weight = 400, .color = t->text_faint, .align = XF_TEXT_RIGHT
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

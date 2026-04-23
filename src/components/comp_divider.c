#include "comp_divider.h"
#include "draw.h"

static void draw(xf_draw_ctx_t *ctx, int w, int h, void *user_data)
{
    const xf_theme_t *t = xf_get_theme();
    (void)h; (void)user_data;

    xf_draw_begin_path(ctx);
    xf_draw_move_to(ctx, 0,       0.5);
    xf_draw_line_to(ctx, (double)w, 0.5);
    xf_draw_stroke(ctx, t->surface_separator, 1.0, XF_LINE_CAP_BUTT);
}

static void render(xf_component_t *self, uint8_t *buf, int w, int h)
{
    (void)self;
    xf_render(buf, w, h, draw, NULL);
}

xf_component_t comp_divider_create(void)
{
    xf_component_t c = XF_COMPONENT(render);
    return c;
}

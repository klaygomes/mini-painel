/**
 * @file comp_spacer.c
 * @brief Empty vertical gap — no drawing, just an opaque transparent region.
 */

#include "comp_spacer.h"
#include "draw.h"

static void draw(xf_draw_ctx_t *ctx, int w, int h, void *user_data)
{
    (void)ctx; (void)w; (void)h; (void)user_data;
    /* xf_render already clears the surface to transparent before calling here */
}

static void render(xf_component_t *self, uint8_t *buf, int w, int h)
{
    (void)self;
    xf_render(buf, w, h, draw, NULL);
}

xf_component_t comp_spacer_create(void)
{
    xf_component_t c = XF_COMPONENT(render);
    return c;
}

#ifndef XF_GFX_PILL_H
#define XF_GFX_PILL_H

#include "gfx.h"

double xf_gfx_pill_width(xf_draw_ctx_t *ctx, const char *text, int weight);
void xf_gfx_pill(xf_draw_ctx_t *ctx, double cx, double cy,
                  const char *text, int weight, xf_rgba_t bg, xf_rgba_t fg);

#endif /* XF_GFX_PILL_H */

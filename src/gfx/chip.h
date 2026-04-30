#ifndef XF_GFX_CHIP_H
#define XF_GFX_CHIP_H

#include "gfx.h"

double xf_gfx_chip_width(xf_draw_ctx_t *ctx, const char *text);
void xf_gfx_chip(xf_draw_ctx_t *ctx, double x, double cy,
                  const char *text, xf_rgba_t bg, xf_rgba_t fg);

#endif /* XF_GFX_CHIP_H */

#ifndef XF_GFX_PROGRESS_BAR_H
#define XF_GFX_PROGRESS_BAR_H

#include "gfx.h"

void xf_gfx_progress_bar(xf_draw_ctx_t *ctx,
                           double x, double y, double w, double h,
                           double fraction, xf_rgba_t fill);

#endif /* XF_GFX_PROGRESS_BAR_H */

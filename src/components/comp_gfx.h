#pragma once

#include "draw.h"
#include "layout.h"

/* Muted semibold text label at the top of a component section. */
void xf_gfx_section_label(xf_draw_ctx_t *ctx, const char *text, double y);

/* Filled circle — status indicator or severity dot. */
void xf_gfx_dot(xf_draw_ctx_t *ctx, double cx, double cy, double r, xf_rgba_t color);

/* Online-presence indicator: white halo + colored inner circle. */
void xf_gfx_online_dot(xf_draw_ctx_t *ctx, double cx, double cy, xf_rgba_t color);

/* Colored circle with centered initials text. Font size is chosen from radius. */
void xf_gfx_avatar(xf_draw_ctx_t *ctx, double cx, double cy, double r,
                    xf_rgba_t color, const char *initials);

/* Auto-width pill (h=14, r=7) with centered FONT_SM text.
   cx and cy are the horizontal and vertical centre of the pill. */
void xf_gfx_pill(xf_draw_ctx_t *ctx, double cx, double cy,
                  const char *text, int weight, xf_rgba_t bg, xf_rgba_t fg);

/* Pill width for the given text and weight — use for positioning adjacent elements. */
double xf_gfx_pill_width(xf_draw_ctx_t *ctx, const char *text, int weight);

/* Monospace chip (h=16, r=8) with left-padded text.
   x is the left edge, cy is the vertical centre. */
void xf_gfx_chip(xf_draw_ctx_t *ctx, double x, double cy,
                  const char *text, xf_rgba_t bg, xf_rgba_t fg);

/* Chip width — use for positioning elements after the chip. */
double xf_gfx_chip_width(xf_draw_ctx_t *ctx, const char *text);

/* Checkbox (size=11px): checked = filled square + white checkmark glyph;
   unchecked = outlined square.  x is the left edge, cy is the vertical centre. */
void xf_gfx_checkbox(xf_draw_ctx_t *ctx, double x, double cy, int checked);

/* Filled circle with a white checkmark glyph inside. */
void xf_gfx_icon_check(xf_draw_ctx_t *ctx, double cx, double cy, double r,
                         xf_rgba_t color);

/* Progress bar: grey track + clamped colored fill. fraction in [0.0, 1.0]. */
void xf_gfx_progress_bar(xf_draw_ctx_t *ctx,
                           double x, double y, double w, double h,
                           double fraction, xf_rgba_t fill);

/* Horizontal strikethrough line at baseline - font_size*0.35, width pixels wide. */
void xf_gfx_strikethrough(xf_draw_ctx_t *ctx,
                            double x, double baseline,
                            double width, double font_size);

/* State-colored dot; color is resolved from theme by status. */
typedef enum {
    XF_STATUS_INPUT   = 0,
    XF_STATUS_RUNNING,
    XF_STATUS_IDLE,
    XF_STATUS_DONE
} xf_status_t;

void xf_gfx_status_icon(xf_draw_ctx_t *ctx, double cx, double cy, double r,
                          xf_status_t status);

/**
 * @file draw.h
 * @brief Engine-agnostic 2-D drawing API used by all dashboard components.
 *
 * Components include only this header. The Cairo implementation lives in
 * draw.c and is the only file that references cairo.h. Swapping the rendering
 * backend requires replacing draw.c; component code is unaffected.
 *
 * Typical component usage:
 * @code
 *   static void draw(xf_draw_ctx_t *ctx, void *user_data) {
 *       const xf_theme_t       *t = xf_get_theme();
 *       const comp_foo_data_t  *d = user_data;
 *       xf_draw_fill_round_rect(ctx, 0, 0, w, h, 6, t->surface_card);
 *       xf_draw_text(ctx, d->label, 8, 14, &(xf_text_opts_t){
 *           .size = FONT_MD, .weight = WEIGHT_MEDIUM, .color = t->text_primary
 *       });
 *   }
 *
 *   static void render(xf_component_t *self, uint8_t *buf, int w, int h) {
 *       xf_render(buf, w, h, draw, self->ctx);
 *   }
 * @endcode
 */

#ifndef DRAW_H
#define DRAW_H

#include "theme.h"

#include <stdint.h>

/**
 * @brief Horizontal alignment anchor for xf_draw_text.
 *
 * The x coordinate passed to xf_draw_text is interpreted relative to
 * the chosen alignment: LEFT means x is the left edge, CENTER means x is the
 * midpoint, RIGHT means x is the right edge.
 */
typedef enum {
    XF_TEXT_LEFT   = 0,
    XF_TEXT_CENTER,
    XF_TEXT_RIGHT
} xf_text_align_t;

/**
 * @brief Visual properties for a single xf_draw_text call.
 *
 * Zero-initialise with a compound literal and set only the fields you need.
 * Always set size and color explicitly; unset color defaults to
 * transparent black, which produces invisible text.
 */
typedef struct {
    double          size;      /**< Font size in pixels (required)                  */
    int             weight;    /**< Font weight: 400 = normal, >400 = bold          */
    xf_rgba_t       color;     /**< Text colour (required)                          */
    const char     *family;    /**< Font family; NULL uses theme->font_sans          */
    xf_text_align_t align;     /**< Alignment anchor for the x coordinate           */
    double          max_width; /**< Clip text after this many pixels; 0 = no clip   */
} xf_text_opts_t;

/**
 * @brief Line-end cap style for xf_draw_stroke.
 */
typedef enum {
    XF_LINE_CAP_BUTT   = 0,
    XF_LINE_CAP_ROUND,
    XF_LINE_CAP_SQUARE
} xf_line_cap_t;

/**
 * @brief Opaque draw context passed to every draw primitive and callback.
 *
 * Components receive this through the xf_draw_fn_t callback. They must
 * not allocate, free, or inspect the internals — it is owned by xf_render.
 */
typedef struct xf_draw_ctx xf_draw_ctx_t;

/**
 * @brief Fill a rounded rectangle with a solid colour.
 *
 * @param ctx    Draw context.
 * @param x, y   Top-left corner in pixels.
 * @param w, h   Width and height in pixels.
 * @param r      Corner radius in pixels; 0 produces a plain rectangle.
 * @param color  Fill colour.
 */
void xf_draw_fill_round_rect(xf_draw_ctx_t *ctx,
                              double x, double y, double w, double h,
                              double r, xf_rgba_t color);

/**
 * @brief Stroke the outline of a rounded rectangle.
 *
 * @param ctx        Draw context.
 * @param x, y       Top-left corner in pixels.
 * @param w, h       Width and height in pixels.
 * @param r          Corner radius; 0 produces a plain rectangle.
 * @param color      Stroke colour.
 * @param line_width Stroke width in pixels.
 */
void xf_draw_stroke_round_rect(xf_draw_ctx_t *ctx,
                                double x, double y, double w, double h,
                                double r, xf_rgba_t color, double line_width);

/**
 * @brief Fill a circle with a solid colour.
 *
 * @param ctx      Draw context.
 * @param cx, cy   Centre coordinates in pixels.
 * @param r        Radius in pixels.
 * @param color    Fill colour.
 */
void xf_draw_circle(xf_draw_ctx_t *ctx,
                    double cx, double cy, double r,
                    xf_rgba_t color);

/**
 * @brief Draw a UTF-8 text string.
 *
 * y is the baseline. Alignment and clipping are controlled through opts.
 *
 * @param ctx   Draw context.
 * @param text  UTF-8 encoded string.
 * @param x     Horizontal anchor (interpretation depends on opts->align).
 * @param y     Baseline y position in pixels.
 * @param opts  Visual properties; must not be NULL.
 */
void xf_draw_text(xf_draw_ctx_t *ctx, const char *text,
                  double x, double y,
                  const xf_text_opts_t *opts);

/**
 * @brief Measure the advance width of a string in pixels.
 *
 * @param ctx     Draw context.
 * @param text    UTF-8 string to measure.
 * @param size    Font size in pixels.
 * @param weight  Font weight (400 = normal, >400 = bold).
 * @return        Advance width in pixels.
 */
double xf_draw_measure_text(xf_draw_ctx_t *ctx, const char *text,
                             double size, int weight);

/** @brief Start a new path, discarding any current path state. */
void xf_draw_begin_path(xf_draw_ctx_t *ctx);

/** @brief Move the current point without drawing. */
void xf_draw_move_to(xf_draw_ctx_t *ctx, double x, double y);

/** @brief Append a straight line from the current point to (x, y). */
void xf_draw_line_to(xf_draw_ctx_t *ctx, double x, double y);

/**
 * @brief Close the current sub-path by adding a line back to its start.
 *
 * Required before xf_draw_fill to ensure a closed, fillable shape.
 */
void xf_draw_close_path(xf_draw_ctx_t *ctx);

/**
 * @brief Fill the current path and clear it.
 *
 * @param ctx    Draw context.
 * @param color  Fill colour.
 */
void xf_draw_fill(xf_draw_ctx_t *ctx, xf_rgba_t color);

/**
 * @brief Stroke the current path and clear it.
 *
 * @param ctx        Draw context.
 * @param color      Stroke colour.
 * @param line_width Stroke width in pixels.
 * @param cap        Line-end cap style.
 */
void xf_draw_stroke(xf_draw_ctx_t *ctx, xf_rgba_t color,
                    double line_width, xf_line_cap_t cap);

/** Return the pixel width of the canvas backing this context. */
int xf_draw_width(const xf_draw_ctx_t *ctx);

/** Return the pixel height of the canvas backing this context. */
int xf_draw_height(const xf_draw_ctx_t *ctx);

/**
 * @brief Callback signature for component draw functions.
 *
 * @param ctx        Draw context owned by xf_render; do not store it.
 * @param user_data  Caller-supplied context (the component's data pointer).
 */
typedef void (*xf_draw_fn_t)(xf_draw_ctx_t *ctx, void *user_data);

/**
 * @brief Render a component into a raw RGB888 buffer.
 *
 * Creates a temporary backing surface, calls fn, then converts and writes
 * the result into buf as packed RGB888 (width * height * 3 bytes,
 * row-major, top-to-bottom). The surface is destroyed after the call.
 *
 * @param buf        Destination; must be at least w * h * 3 bytes.
 * @param w, h       Pixel dimensions.
 * @param fn         Draw callback; must not be NULL.
 * @param user_data  Forwarded unchanged to fn.
 */
void xf_render(uint8_t *buf, int w, int h, xf_draw_fn_t fn, void *user_data);

/* Fill an RGB888 buffer with a solid color. */
void xf_fill_rgb888(uint8_t *buf, int w, int h, xf_rgba_t color);

#endif /* DRAW_H */

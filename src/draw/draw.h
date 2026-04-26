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
 *   static void draw(xf_draw_ctx_t *ctx, int w, int h, void *user_data) {
 *       const xf_theme_t       *t = xf_get_theme();
 *       const comp_foo_data_t  *d = user_data;
 *       xf_draw_fill_round_rect(ctx, 0, 0, w, h, 6, t->surface_card);
 *       xf_draw_text(ctx, d->label, 8, 14, &(xf_text_opts_t){
 *           .size = 11, .weight = 500, .color = t->text_primary
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

#include <stddef.h>
#include <stdint.h>

/**
 * @brief RGBA color with normalized channel values in [0.0, 1.0].
 *
 * Using normalized doubles keeps the type independent of any rendering API.
 * Use @ref XF_RGB or @ref XF_RGBA to initialise values from 0xRRGGBB literals.
 */
typedef struct {
    double r; /**< Red   channel [0.0, 1.0] */
    double g; /**< Green channel [0.0, 1.0] */
    double b; /**< Blue  channel [0.0, 1.0] */
    double a; /**< Alpha channel [0.0, 1.0] (1.0 = fully opaque) */
} xf_rgba_t;

/**
 * @brief Initialise an @ref xf_rgba_t from a 0xRRGGBB integer (alpha = 1.0).
 *
 * For use in aggregate initialisers only, not as function arguments.
 *
 * @param hex  24-bit colour packed as 0xRRGGBB.
 */
#define XF_RGB(hex) \
    { ((hex) >> 16 & 0xFF) / 255.0, \
      ((hex) >>  8 & 0xFF) / 255.0, \
      ((hex)       & 0xFF) / 255.0, \
      1.0 }

/**
 * @brief Initialise an @ref xf_rgba_t from a 0xRRGGBB integer with custom alpha.
 *
 * For use in aggregate initialisers only, not as function arguments.
 *
 * @param hex    24-bit colour packed as 0xRRGGBB.
 * @param alpha  Opacity in [0.0, 1.0].
 */
#define XF_RGBA(hex, alpha) \
    { ((hex) >> 16 & 0xFF) / 255.0, \
      ((hex) >>  8 & 0xFF) / 255.0, \
      ((hex)       & 0xFF) / 255.0, \
      (alpha) }

/**
 * @brief Complete colour and typography palette for all dashboard components.
 *
 * All components read colours exclusively through the active theme. No colour
 * literal may appear in component source; every colour is a field reference
 * such as @c t->danger. This ensures a full visual retheme requires only a
 * new @ref xf_theme_t value passed to @ref xf_set_theme.
 *
 * Font names are plain C strings understood by most 2-D text engines.
 */
typedef struct {
    xf_rgba_t background;     /**< Page and component background (#FFFFFF) */
    xf_rgba_t text_primary;   /**< Main content text           (#1a1a1a) */
    xf_rgba_t text_secondary; /**< Names, subtitles            (#444441) */
    xf_rgba_t text_muted;     /**< Section labels, headers     (#6b6b67) */
    xf_rgba_t text_faint;     /**< Timestamps, minor info      (#9a9a95) */
    xf_rgba_t text_dimmed;    /**< Secondary values            (#5F5E5A) */

    xf_rgba_t surface_card;      /**< Card / chip background     (#f1efe8)         */
    xf_rgba_t surface_separator; /**< Thin divider line          (rgba 0,0,0,0.08) */
    xf_rgba_t surface_border;    /**< Subtle border / avatar bg  (#D3D1C7)         */
    xf_rgba_t white;             /**< Pure white (marks, initials)(#FFFFFF)        */
    xf_rgba_t offline;           /**< Offline / inactive state   (#B4B2A9)         */

    xf_rgba_t danger;           /**< Foreground / dot   (#E24B4A) */
    xf_rgba_t danger_bg;        /**< Row background     (#FCEBEB) */
    xf_rgba_t danger_pill_bg;   /**< Pill fill          (#F7C1C1) */
    xf_rgba_t danger_pill_fg;   /**< Pill text & dur fg (#791F1F) */
    xf_rgba_t danger_title_fg;  /**< Row title text     (#501313) */

    xf_rgba_t warning;           /**< Foreground / dot   (#EF9F27) */
    xf_rgba_t warning_bg;        /**< Row background     (#FAEEDA) */
    xf_rgba_t warning_pill_bg;   /**< Pill fill          (#FAC775) */
    xf_rgba_t warning_pill_fg;   /**< Pill text & dur fg (#633806) */
    xf_rgba_t warning_title_fg;  /**< Row title text     (#412402) */

    xf_rgba_t success;    /**< Foreground / bar   (#1D9E75) */
    xf_rgba_t success_bg; /**< Background tint    (#EAF3DE) */
    xf_rgba_t success_fg; /**< Text on success bg (#27500A) */

    xf_rgba_t info;      /**< Foreground / bar       (#378ADD)           */
    xf_rgba_t info_bg;   /**< Background tint        (#E6F1FB)           */
    xf_rgba_t info_fg;   /**< Text on info bg        (#185FA5)           */
    xf_rgba_t info_dark; /**< Strong info text       (#042C53)           */
    xf_rgba_t info_fill; /**< Sparkline area fill    (rgba 378ADD, 0.14) */

    xf_rgba_t accent;    /**< Foreground         (#7F77DD) */
    xf_rgba_t accent_bg; /**< Chip background    (#EEEDFE) */
    xf_rgba_t accent_fg; /**< Text on accent bg  (#3C3489) */

    xf_rgba_t orange;           /**< Schedule / team bar    (#D85A30)           */
    xf_rgba_t purple_bar;       /**< Sprint progress fill   (#534AB7)           */
    xf_rgba_t deploy_bar;       /**< Deploy check circle    (#639922)           */
    xf_rgba_t deploy_chip_bg;   /**< Branch chip fill       (rgba 639922, 0.18) */
    xf_rgba_t deploy_text;      /**< Deploy time-ago text   (#3B6D11)           */
    xf_rgba_t deploy_text_dark; /**< Deploy branch/label    (#173404)           */

    const char *font_sans; /**< Proportional face, e.g. "sans-serif" */
    const char *font_mono; /**< Monospaced face,    e.g. "monospace"  */
} xf_theme_t;

/**
 * @brief Built-in light theme matching the original design palette.
 *
 * Pass to @ref xf_set_theme before the first render call. Define a new
 * @ref xf_theme_t and call @ref xf_set_theme again to switch themes at runtime.
 */
extern const xf_theme_t xf_theme_default;

/**
 * @brief Set the active theme for all subsequent draw operations.
 *
 * Not thread-safe; intended to be called once at startup before any render.
 * The pointer must remain valid for the lifetime of the program.
 *
 * @param theme  Theme to activate. Must not be NULL.
 */
void xf_set_theme(const xf_theme_t *theme);

/**
 * @brief Return the currently active theme.
 *
 * Never returns NULL; falls back to @ref xf_theme_default if @ref xf_set_theme
 * was never called.
 */
const xf_theme_t *xf_get_theme(void);

/**
 * @brief Horizontal alignment anchor for @ref xf_draw_text.
 *
 * The @c x coordinate passed to @ref xf_draw_text is interpreted relative to
 * the chosen alignment: LEFT means x is the left edge, CENTER means x is the
 * midpoint, RIGHT means x is the right edge.
 */
typedef enum {
    XF_TEXT_LEFT   = 0,
    XF_TEXT_CENTER,
    XF_TEXT_RIGHT
} xf_text_align_t;

/**
 * @brief Visual properties for a single @ref xf_draw_text call.
 *
 * Zero-initialise with a compound literal and set only the fields you need.
 * Always set @c size and @c color explicitly; unset @c color defaults to
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
 * @brief Line-end cap style for @ref xf_draw_stroke.
 */
typedef enum {
    XF_LINE_CAP_BUTT   = 0,
    XF_LINE_CAP_ROUND,
    XF_LINE_CAP_SQUARE
} xf_line_cap_t;

/**
 * @brief Opaque draw context passed to every draw primitive and callback.
 *
 * Components receive this through the @ref xf_draw_fn_t callback. They must
 * not allocate, free, or inspect the internals — it is owned by @ref xf_render.
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
 * @c y is the baseline, matching the original canvas convention. Alignment
 * and clipping are controlled through @p opts.
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
 * Used to compute right-aligned positions and chip widths before drawing.
 *
 * @param ctx     Draw context.
 * @param text    UTF-8 string to measure.
 * @param size    Font size in pixels.
 * @param weight  Font weight (400 = normal, >400 = bold).
 * @return        Advance width in pixels.
 */
double xf_draw_measure_text(xf_draw_ctx_t *ctx, const char *text,
                             double size, int weight);

/**
 * @brief Start a new path, discarding any current path state.
 */
void xf_draw_begin_path(xf_draw_ctx_t *ctx);

/** @brief Move the current point without drawing. */
void xf_draw_move_to(xf_draw_ctx_t *ctx, double x, double y);

/** @brief Append a straight line from the current point to (x, y). */
void xf_draw_line_to(xf_draw_ctx_t *ctx, double x, double y);

/**
 * @brief Close the current sub-path by adding a line back to its start.
 *
 * Required before @ref xf_draw_fill to ensure a closed, fillable shape.
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

/**
 * @brief Callback signature for component draw functions.
 *
 * @param ctx        Draw context owned by @ref xf_render; do not store it.
 * @param w, h       Pixel dimensions of the component region.
 * @param user_data  Caller-supplied context (the component's data pointer).
 */
typedef void (*xf_draw_fn_t)(xf_draw_ctx_t *ctx, int w, int h,
                              void *user_data);

/**
 * @brief Render a component into a raw RGB888 buffer.
 *
 * Creates a temporary backing surface, calls @p fn, then converts and writes
 * the result into @p buf as packed RGB888 (width * height * 3 bytes,
 * row-major, top-to-bottom). The surface is destroyed after the call.
 *
 * @param buf        Destination; must be at least w * h * 3 bytes.
 * @param w, h       Pixel dimensions.
 * @param fn         Draw callback; must not be NULL.
 * @param user_data  Forwarded unchanged to @p fn.
 */
void xf_render(uint8_t *buf, int w, int h, xf_draw_fn_t fn, void *user_data);

/* Fill an RGB888 buffer with a solid color. */
void xf_fill_rgb888(uint8_t *buf, int w, int h, xf_rgba_t color);

#endif /* DRAW_H */

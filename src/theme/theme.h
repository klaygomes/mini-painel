#ifndef THEME_H
#define THEME_H

#include <stddef.h>

/**
 * RGBA color with normalized channel values in [0.0, 1.0].
 *
 * Use XF_RGB or XF_RGBA to initialise values from 0xRRGGBB literals.
 */
typedef struct {
    double r;
    double g;
    double b;
    double a;
} xf_rgba_t;

/** Initialise an xf_rgba_t from a 0xRRGGBB integer (alpha = 1.0). */
#define XF_RGB(hex) \
    { ((hex) >> 16 & 0xFF) / 255.0, \
      ((hex) >>  8 & 0xFF) / 255.0, \
      ((hex)       & 0xFF) / 255.0, \
      1.0 }

/** Initialise an xf_rgba_t from a 0xRRGGBB integer with custom alpha. */
#define XF_RGBA(hex, alpha) \
    { ((hex) >> 16 & 0xFF) / 255.0, \
      ((hex) >>  8 & 0xFF) / 255.0, \
      ((hex)       & 0xFF) / 255.0, \
      (alpha) }

/**
 * Complete colour and typography palette for all dashboard components.
 *
 * No colour literal may appear in component source — every colour is a field
 * reference such as t->danger. A full retheme requires only a new xf_theme_t
 * value passed to xf_set_theme.
 */
typedef struct {
    xf_rgba_t background;
    xf_rgba_t text_primary;
    xf_rgba_t text_secondary;
    xf_rgba_t text_muted;
    xf_rgba_t text_faint;
    xf_rgba_t text_dimmed;

    xf_rgba_t surface_card;
    xf_rgba_t surface_separator;
    xf_rgba_t surface_border;
    xf_rgba_t white;
    xf_rgba_t offline;

    xf_rgba_t danger;
    xf_rgba_t danger_bg;
    xf_rgba_t danger_pill_bg;
    xf_rgba_t danger_pill_fg;
    xf_rgba_t danger_title_fg;

    xf_rgba_t warning;
    xf_rgba_t warning_bg;
    xf_rgba_t warning_pill_bg;
    xf_rgba_t warning_pill_fg;
    xf_rgba_t warning_title_fg;

    xf_rgba_t success;
    xf_rgba_t success_bg;
    xf_rgba_t success_fg;

    xf_rgba_t info;
    xf_rgba_t info_bg;
    xf_rgba_t info_fg;
    xf_rgba_t info_dark;
    xf_rgba_t info_fill;

    xf_rgba_t accent;
    xf_rgba_t accent_bg;
    xf_rgba_t accent_fg;

    xf_rgba_t orange;
    xf_rgba_t purple_bar;
    xf_rgba_t deploy_bar;
    xf_rgba_t deploy_chip_bg;
    xf_rgba_t deploy_text;
    xf_rgba_t deploy_text_dark;

    const char *font_sans;
    const char *font_mono;
} xf_theme_t;

/**
 * Built-in light theme matching the original design palette.
 *
 * Which theme_*.c file defines this symbol is controlled at build time by the
 * XF_THEME CMake cache variable (default: "default").  Pass -DXF_THEME=dark to
 * cmake to compile theme_dark.c instead.
 */
extern const xf_theme_t xf_theme_default;

/**
 * Set the active theme for all subsequent draw operations.
 *
 * Not thread-safe; intended to be called once at startup.
 * The pointer must remain valid for the lifetime of the program.
 */
void xf_set_theme(const xf_theme_t *theme);

/** Return the currently active theme. Never returns NULL. */
const xf_theme_t *xf_get_theme(void);

/* Font sizes in pixels. Change here (or in a custom theme_*.h) to rescale all
 * components without touching individual draw calls. */
#define FONT_XS    9   /* initials, badges, small indicators */
#define FONT_SM   10   /* timestamps, secondary labels */
#define FONT_MD   11   /* body text, section labels */
#define FONT_LG   12   /* primary content, titles */
#define FONT_XL   13   /* prominent names */
#define FONT_HERO 14   /* large metric values */

/* Font weights (CSS-style numeric scale). */
#define WEIGHT_NORMAL   400
#define WEIGHT_MEDIUM   500
#define WEIGHT_SEMIBOLD 600
#define WEIGHT_BOLD     700

#endif /* THEME_H */

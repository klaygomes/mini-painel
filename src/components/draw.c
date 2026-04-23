/* This is the only file in the project that includes cairo.h. Replacing this
 * file with a different backend is sufficient to change the rendering engine;
 * all component code remains unchanged. */

#include "draw.h"

#include <cairo.h>
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

const xf_theme_t xf_theme_default = {
    .text_primary   = XF_RGB(0x1a1a1a),
    .text_secondary = XF_RGB(0x444441),
    .text_muted     = XF_RGB(0x6b6b67),
    .text_faint     = XF_RGB(0x9a9a95),
    .text_dimmed    = XF_RGB(0x5F5E5A),

    .surface_card      = XF_RGB(0xf1efe8),
    .surface_separator = XF_RGBA(0x000000, 0.08),
    .surface_border    = XF_RGB(0xD3D1C7),
    .white             = XF_RGB(0xFFFFFF),
    .offline           = XF_RGB(0xB4B2A9),

    .danger          = XF_RGB(0xE24B4A),
    .danger_bg       = XF_RGB(0xFCEBEB),
    .danger_pill_bg  = XF_RGB(0xF7C1C1),
    .danger_pill_fg  = XF_RGB(0x791F1F),
    .danger_title_fg = XF_RGB(0x501313),

    .warning          = XF_RGB(0xEF9F27),
    .warning_bg       = XF_RGB(0xFAEEDA),
    .warning_pill_bg  = XF_RGB(0xFAC775),
    .warning_pill_fg  = XF_RGB(0x633806),
    .warning_title_fg = XF_RGB(0x412402),

    .success    = XF_RGB(0x1D9E75),
    .success_bg = XF_RGB(0xEAF3DE),
    .success_fg = XF_RGB(0x27500A),

    .info      = XF_RGB(0x378ADD),
    .info_bg   = XF_RGB(0xE6F1FB),
    .info_fg   = XF_RGB(0x185FA5),
    .info_dark = XF_RGB(0x042C53),
    .info_fill = XF_RGBA(0x378ADD, 0.14),

    .accent    = XF_RGB(0x7F77DD),
    .accent_bg = XF_RGB(0xEEEDFE),
    .accent_fg = XF_RGB(0x3C3489),

    .orange           = XF_RGB(0xD85A30),
    .purple_bar       = XF_RGB(0x534AB7),
    .deploy_bar       = XF_RGB(0x639922),
    .deploy_chip_bg   = XF_RGBA(0x639922, 0.18),
    .deploy_text      = XF_RGB(0x3B6D11),
    .deploy_text_dark = XF_RGB(0x173404),

    .font_sans = "sans-serif",
    .font_mono = "monospace",
};

static const xf_theme_t *g_theme = &xf_theme_default;

void xf_set_theme(const xf_theme_t *theme)
{
    g_theme = theme;
}

const xf_theme_t *xf_get_theme(void)
{
    return g_theme;
}

struct xf_draw_ctx {
    cairo_t *cr;
};

static void set_color(cairo_t *cr, xf_rgba_t c)
{
    cairo_set_source_rgba(cr, c.r, c.g, c.b, c.a);
}

static void round_rect_path(cairo_t *cr,
                              double x, double y, double w, double h,
                              double r)
{
    if (r <= 0) {
        cairo_rectangle(cr, x, y, w, h);
        return;
    }
    /* clamp so arcs never overlap for very small rectangles */
    if (r > w / 2.0) r = w / 2.0;
    if (r > h / 2.0) r = h / 2.0;

    cairo_arc(cr, x + w - r, y + r,     r, -M_PI / 2.0,        0.0);
    cairo_arc(cr, x + w - r, y + h - r, r,  0.0,                M_PI / 2.0);
    cairo_arc(cr, x + r,     y + h - r, r,  M_PI / 2.0,         M_PI);
    cairo_arc(cr, x + r,     y + r,     r,  M_PI,               3.0 * M_PI / 2.0);
    cairo_close_path(cr);
}

/* Cairo ARGB32 on little-endian stores bytes as [B, G, R, A] per pixel. */
static void surface_to_rgb888(cairo_surface_t *surf, uint8_t *buf, int w, int h)
{
    cairo_surface_flush(surf);
    const unsigned char *data   = cairo_image_surface_get_data(surf);
    int                  stride = cairo_image_surface_get_stride(surf);

    for (int row = 0; row < h; row++) {
        const uint8_t *src = data + row * stride;
        uint8_t       *dst = buf  + row * w * 3;
        for (int col = 0; col < w; col++) {
            dst[col * 3 + 0] = src[col * 4 + 2];
            dst[col * 3 + 1] = src[col * 4 + 1];
            dst[col * 3 + 2] = src[col * 4 + 0];
        }
    }
}

static void select_font(cairo_t *cr, const char *family, int weight, double size)
{
    cairo_select_font_face(cr, family,
                           CAIRO_FONT_SLANT_NORMAL,
                           weight > 400
                               ? CAIRO_FONT_WEIGHT_BOLD
                               : CAIRO_FONT_WEIGHT_NORMAL);
    cairo_set_font_size(cr, size);
}

void xf_draw_fill_round_rect(xf_draw_ctx_t *ctx,
                              double x, double y, double w, double h,
                              double r, xf_rgba_t color)
{
    cairo_new_path(ctx->cr);
    round_rect_path(ctx->cr, x, y, w, h, r);
    set_color(ctx->cr, color);
    cairo_fill(ctx->cr);
}

void xf_draw_stroke_round_rect(xf_draw_ctx_t *ctx,
                                double x, double y, double w, double h,
                                double r, xf_rgba_t color, double line_width)
{
    cairo_new_path(ctx->cr);
    round_rect_path(ctx->cr, x, y, w, h, r);
    set_color(ctx->cr, color);
    cairo_set_line_width(ctx->cr, line_width);
    cairo_stroke(ctx->cr);
}

void xf_draw_circle(xf_draw_ctx_t *ctx,
                    double cx, double cy, double r,
                    xf_rgba_t color)
{
    cairo_new_path(ctx->cr);
    cairo_arc(ctx->cr, cx, cy, r, 0.0, 2.0 * M_PI);
    set_color(ctx->cr, color);
    cairo_fill(ctx->cr);
}

void xf_draw_text(xf_draw_ctx_t *ctx, const char *text,
                  double x, double y,
                  const xf_text_opts_t *opts)
{
    const xf_theme_t *t      = xf_get_theme();
    const char       *family = opts->family ? opts->family : t->font_sans;

    select_font(ctx->cr, family, opts->weight, opts->size);

    cairo_text_extents_t ext;
    cairo_text_extents(ctx->cr, text, &ext);

    double tx = x;
    if (opts->align == XF_TEXT_CENTER)
        tx = x - ext.x_advance / 2.0;
    else if (opts->align == XF_TEXT_RIGHT)
        tx = x - ext.x_advance;

    if (opts->max_width > 0) {
        cairo_save(ctx->cr);
        /* generous vertical bounds because font metrics vary across engines */
        cairo_rectangle(ctx->cr, tx, y - opts->size * 1.5,
                        opts->max_width, opts->size * 3.0);
        cairo_clip(ctx->cr);
    }

    set_color(ctx->cr, opts->color);
    cairo_move_to(ctx->cr, tx, y);
    cairo_show_text(ctx->cr, text);

    if (opts->max_width > 0)
        cairo_restore(ctx->cr);
}

double xf_draw_measure_text(xf_draw_ctx_t *ctx, const char *text,
                             double size, int weight)
{
    const xf_theme_t *t = xf_get_theme();
    select_font(ctx->cr, t->font_sans, weight, size);
    cairo_text_extents_t ext;
    cairo_text_extents(ctx->cr, text, &ext);
    return ext.x_advance;
}

void xf_draw_begin_path(xf_draw_ctx_t *ctx)
{
    cairo_new_path(ctx->cr);
}

void xf_draw_move_to(xf_draw_ctx_t *ctx, double x, double y)
{
    cairo_move_to(ctx->cr, x, y);
}

void xf_draw_line_to(xf_draw_ctx_t *ctx, double x, double y)
{
    cairo_line_to(ctx->cr, x, y);
}

void xf_draw_close_path(xf_draw_ctx_t *ctx)
{
    cairo_close_path(ctx->cr);
}

void xf_draw_fill(xf_draw_ctx_t *ctx, xf_rgba_t color)
{
    set_color(ctx->cr, color);
    cairo_fill(ctx->cr);
}

void xf_draw_stroke(xf_draw_ctx_t *ctx, xf_rgba_t color,
                    double line_width, xf_line_cap_t cap)
{
    set_color(ctx->cr, color);
    cairo_set_line_width(ctx->cr, line_width);

    cairo_line_cap_t cairo_cap;
    switch (cap) {
        case XF_LINE_CAP_ROUND:  cairo_cap = CAIRO_LINE_CAP_ROUND;  break;
        case XF_LINE_CAP_SQUARE: cairo_cap = CAIRO_LINE_CAP_SQUARE; break;
        default:                 cairo_cap = CAIRO_LINE_CAP_BUTT;   break;
    }
    cairo_set_line_cap(ctx->cr, cairo_cap);
    cairo_stroke(ctx->cr);
}

void xf_render(uint8_t *buf, int w, int h, xf_draw_fn_t fn, void *user_data)
{
    cairo_surface_t *surf = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, w, h);
    cairo_t         *cr   = cairo_create(surf);

    /* transparent background so components without full coverage composite
     * correctly when composited onto a coloured dashboard background */
    cairo_set_source_rgba(cr, 0, 0, 0, 0);
    cairo_paint(cr);

    xf_draw_ctx_t ctx = { cr };
    fn(&ctx, w, h, user_data);

    surface_to_rgb888(surf, buf, w, h);

    cairo_destroy(cr);
    cairo_surface_destroy(surf);
}

#include "comp_gfx.h"

void xf_gfx_section_label(xf_draw_ctx_t *ctx, const char *text, double y)
{
    const xf_theme_t *t = xf_get_theme();
    xf_draw_text(ctx, text, LAY_PAD_X, y, &(xf_text_opts_t){
        .size = FONT_MD, .weight = WEIGHT_SEMIBOLD, .color = t->text_muted
    });
}

void xf_gfx_dot(xf_draw_ctx_t *ctx, double cx, double cy, double r, xf_rgba_t color)
{
    xf_draw_circle(ctx, cx, cy, r, color);
}

void xf_gfx_online_dot(xf_draw_ctx_t *ctx, double cx, double cy, xf_rgba_t color)
{
    const xf_theme_t *t = xf_get_theme();
    xf_draw_circle(ctx, cx, cy, 4.0, t->white);
    xf_draw_circle(ctx, cx, cy, 2.8, color);
}

void xf_gfx_avatar(xf_draw_ctx_t *ctx, double cx, double cy, double r,
                    xf_rgba_t color, const char *initials)
{
    const xf_theme_t *t = xf_get_theme();
    xf_draw_circle(ctx, cx, cy, r, color);

    double font_size, offset;
    if (r < 10.0) {
        font_size = FONT_XS;
        offset    = 4.5;
    } else if (r <= 12.0) {
        font_size = FONT_MD;
        offset    = 4.0;
    } else {
        font_size = FONT_SM;
        offset    = 5.0;
    }

    xf_draw_text(ctx, initials, cx, cy + offset, &(xf_text_opts_t){
        .size = font_size, .weight = WEIGHT_BOLD, .color = t->white, .align = XF_TEXT_CENTER
    });
}

double xf_gfx_pill_width(xf_draw_ctx_t *ctx, const char *text, int weight)
{
    return xf_draw_measure_text(ctx, text, FONT_SM, weight) + 10.0;
}

void xf_gfx_pill(xf_draw_ctx_t *ctx, double cx, double cy,
                  const char *text, int weight, xf_rgba_t bg, xf_rgba_t fg)
{
    double pw = xf_gfx_pill_width(ctx, text, weight);
    xf_draw_fill_round_rect(ctx, cx - pw / 2.0, cy - 7.0, pw, 14.0, 7.0, bg);
    xf_draw_text(ctx, text, cx, cy + 2.5, &(xf_text_opts_t){
        .size = FONT_SM, .weight = weight, .color = fg, .align = XF_TEXT_CENTER
    });
}

double xf_gfx_chip_width(xf_draw_ctx_t *ctx, const char *text)
{
    return xf_draw_measure_text(ctx, text, FONT_MD, WEIGHT_MEDIUM) + 10.0;
}

void xf_gfx_chip(xf_draw_ctx_t *ctx, double x, double cy,
                  const char *text, xf_rgba_t bg, xf_rgba_t fg)
{
    const xf_theme_t *t = xf_get_theme();
    double cw = xf_gfx_chip_width(ctx, text);
    xf_draw_fill_round_rect(ctx, x, cy - 8.0, cw, 16.0, 8.0, bg);
    xf_draw_text(ctx, text, x + 5.0, cy + 5.0, &(xf_text_opts_t){
        .size = FONT_MD, .weight = WEIGHT_MEDIUM, .color = fg, .family = t->font_mono
    });
}

void xf_gfx_checkbox(xf_draw_ctx_t *ctx, double x, double cy, int checked)
{
    const xf_theme_t *t = xf_get_theme();
    double box_y = cy - 5.5;   /* vertically centre the 11 px box on cy */

    if (checked) {
        xf_draw_fill_round_rect(ctx, x, box_y, 11.0, 11.0, 2.0, t->success);
        xf_draw_begin_path(ctx);
        xf_draw_move_to(ctx, x + 1.5,  box_y + 11.0 * 0.55);
        xf_draw_line_to(ctx, x + 11.0 * 0.42, box_y + 11.0 - 2.0);
        xf_draw_line_to(ctx, x + 11.0 - 1.5,  box_y + 2.0);
        xf_draw_stroke(ctx, t->white, 1.5, XF_LINE_CAP_ROUND);
    } else {
        xf_draw_stroke_round_rect(ctx, x, box_y, 11.0, 11.0, 2.0, t->surface_border, 1.0);
    }
}

void xf_gfx_icon_check(xf_draw_ctx_t *ctx, double cx, double cy, double r,
                         xf_rgba_t color)
{
    const xf_theme_t *t = xf_get_theme();
    xf_draw_circle(ctx, cx, cy, r, color);
    xf_draw_begin_path(ctx);
    xf_draw_move_to(ctx, cx - r * 0.35, cy);
    xf_draw_line_to(ctx, cx - r * 0.05, cy + r * 0.35);
    xf_draw_line_to(ctx, cx + r * 0.40, cy - r * 0.30);
    xf_draw_stroke(ctx, t->white, r * 0.18, XF_LINE_CAP_ROUND);
}

void xf_gfx_progress_bar(xf_draw_ctx_t *ctx,
                           double x, double y, double w, double h,
                           double fraction, xf_rgba_t fill)
{
    const xf_theme_t *t = xf_get_theme();
    xf_draw_fill_round_rect(ctx, x, y, w, h, 3.0, t->surface_card);
    double f  = fraction < 0.0 ? 0.0 : fraction > 1.0 ? 1.0 : fraction;
    double fw = w * f;
    if (fw > 0.0)
        xf_draw_fill_round_rect(ctx, x, y, fw, h, 3.0, fill);
}

void xf_gfx_strikethrough(xf_draw_ctx_t *ctx,
                            double x, double baseline,
                            double width, double font_size)
{
    const xf_theme_t *t = xf_get_theme();
    double sy = baseline - font_size * 0.35;
    xf_draw_begin_path(ctx);
    xf_draw_move_to(ctx, x,         sy);
    xf_draw_line_to(ctx, x + width, sy);
    xf_draw_stroke(ctx, t->text_faint, 1.0, XF_LINE_CAP_BUTT);
}

void xf_gfx_status_icon(xf_draw_ctx_t *ctx, double cx, double cy, double r,
                          xf_status_t status)
{
    const xf_theme_t *t = xf_get_theme();
    xf_rgba_t color;
    switch (status) {
        case XF_STATUS_RUNNING: color = t->info;    break;
        case XF_STATUS_IDLE:    color = t->warning; break;
        case XF_STATUS_DONE:    color = t->success; break;
        default:                color = t->offline; break;
    }
    xf_gfx_dot(ctx, cx, cy, r, color);
}

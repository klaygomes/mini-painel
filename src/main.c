#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "panel.h"
#include "dashboard.h"

#define DISPLAY_W 320
#define DISPLAY_H 480

/* ── demo components ─────────────────────────────────────────────────────── */

/* Header: solid blue bar across the full width. */
static void render_header(xf_component_t *self, uint8_t *buf, int w, int h)
{
    int i;
    (void)self;
    for (i = 0; i < w * h; i++) {
        buf[i*3+0] = 0x1A;
        buf[i*3+1] = 0x6B;
        buf[i*3+2] = 0xC8;
    }
}

/* Left body panel: red. */
static void render_left(xf_component_t *self, uint8_t *buf, int w, int h)
{
    int i;
    (void)self;
    for (i = 0; i < w * h; i++) {
        buf[i*3+0] = 0xC0;
        buf[i*3+1] = 0x20;
        buf[i*3+2] = 0x20;
    }
}

/* Right body panel: green. */
static void render_right(xf_component_t *self, uint8_t *buf, int w, int h)
{
    int i;
    (void)self;
    for (i = 0; i < w * h; i++) {
        buf[i*3+0] = 0x20;
        buf[i*3+1] = 0xA0;
        buf[i*3+2] = 0x40;
    }
}

/* ── main ────────────────────────────────────────────────────────────────── */

int main(void)
{
    xf_dashboard_t *dash;
    xf_component_t  header = XF_COMPONENT(render_header);
    xf_component_t  left   = XF_COMPONENT(render_left);
    xf_component_t  right  = XF_COMPONENT(render_right);
    xf_component_t *body_comps[] = {&left, &right};
    int             body_widths[] = {DISPLAY_W / 2, DISPLAY_W / 2};
    const uint8_t  *frame;
    xf_device_t    *dev;

    /* Build the dashboard layout. */
    dash = dashboard_create(DISPLAY_W, DISPLAY_H);
    if (!dash) {
        fprintf(stderr, "dashboard_create failed\n");
        return 1;
    }

    /* Row 0: full-width header (60 px tall). */
    if (dashboard_add_full_row(dash, &header, 60) < 0) {
        fprintf(stderr, "dashboard_add_full_row failed\n");
        dashboard_destroy(dash);
        return 1;
    }

    /* Row 1: two equal-width body panels filling the rest. */
    if (dashboard_add_row(dash, body_comps, body_widths, 2, DISPLAY_H - 60) < 0) {
        fprintf(stderr, "dashboard_add_row failed\n");
        dashboard_destroy(dash);
        return 1;
    }

    /* Render one frame into the internal RGB888 buffer. */
    frame = dashboard_render(dash);

    /* Open the display and send the frame. */
    dev = panel_open_auto();
    if (!dev) {
        fprintf(stderr, "No XuanFang device found. Is it plugged in?\n");
        dashboard_destroy(dash);
        return 1;
    }

    panel_set_orientation(dev, XF_ORIENT_PORTRAIT);
    panel_set_brightness(dev, 80);

    if (panel_display_bitmap(dev, 0, 0, DISPLAY_W, DISPLAY_H, frame) < 0)
        fprintf(stderr, "panel_display_bitmap failed\n");
    else
        printf("Frame sent.\n");

    panel_close(dev);
    dashboard_destroy(dash);
    return 0;
}

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "panel.h"
#include "dashboard.h"
#include "status.h"

#define DISPLAY_W 320
#define DISPLAY_H 480
#define PADDING     4

static int render_header(xf_component_t *self, uint8_t *buf, int w, int h)
{
    int i;
    (void)self;
    for (i = 0; i < w * h; i++) {
        buf[i*3+0] = 0x1A;
        buf[i*3+1] = 0x6B;
        buf[i*3+2] = 0xC8;
    }
    return XF_RES_OK;
}

static int render_left(xf_component_t *self, uint8_t *buf, int w, int h)
{
    int i;
    (void)self;
    for (i = 0; i < w * h; i++) {
        buf[i*3+0] = 0xC0;
        buf[i*3+1] = 0x20;
        buf[i*3+2] = 0x20;
    }
    return XF_RES_OK;
}

static int render_right(xf_component_t *self, uint8_t *buf, int w, int h)
{
    int i;
    (void)self;
    for (i = 0; i < w * h; i++) {
        buf[i*3+0] = 0x20;
        buf[i*3+1] = 0xA0;
        buf[i*3+2] = 0x40;
    }
    return XF_RES_OK;
}

int main(void)
{
    xf_dashboard_t *dash;
    xf_component_t  header = XF_COMPONENT(render_header);
    xf_component_t  left   = XF_COMPONENT(render_left);
    xf_component_t  right  = XF_COMPONENT(render_right);
    xf_component_t *body_comps[] = {&left, &right};
    int             body_widths[2];
    const uint8_t  *frame;
    xf_device_t    *dev;

    dash = dashboard_create(DISPLAY_W, DISPLAY_H, PADDING);
    if (!dash) {
        fprintf(stderr, "dashboard_create failed\n");
        return 1;
    }

    body_widths[0] = dashboard_content_width(dash) / 2;
    body_widths[1] = dashboard_content_width(dash) / 2;

    if (dashboard_add_full_row(dash, &header, 60) < 0) {
        fprintf(stderr, "dashboard_add_full_row failed\n");
        dashboard_destroy(dash);
        return 1;
    }

    if (dashboard_add_row(dash, body_comps, body_widths, 2, DISPLAY_H - 60) < 0) {
        fprintf(stderr, "dashboard_add_row failed\n");
        dashboard_destroy(dash);
        return 1;
    }

    if (dashboard_render(dash, &frame) < 0) {
        fprintf(stderr, "dashboard_render failed\n");
        dashboard_destroy(dash);
        return 1;
    }

    dev = panel_open_auto();
    if (!dev) {
        fprintf(stderr, "No XuanFang device found. Is it plugged in?\n");
        dashboard_destroy(dash);
        return 1;
    }

    panel_set_orientation(dev, XF_ORIENT_PORTRAIT);
    panel_set_brightness(dev, 80);

    xf_bitmap_t bm = {.x = 0, .y = 0, .width = DISPLAY_W, .height = DISPLAY_H, .rgb888 = frame};
    if (panel_display_bitmap(dev, bm) < 0)
        fprintf(stderr, "panel_display_bitmap failed\n");
    else
        printf("Frame sent.\n");

    panel_close(dev);
    dashboard_destroy(dash);
    return 0;
}

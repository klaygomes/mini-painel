#include "panel.h"
#include "device_internal.h"
#include "panel_common.h"
#include "protocol.h"
#include "port_detect.h"
#include "serial.h"
#include "image.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

static int hello(xf_device_t *dev)
{
    uint8_t payload[8] = {'H', 'E', 'L', 'L', 'O', 0, 0, 0};

    if (proto_send_cmd(dev->base.fd, CMD_HELLO, payload) < 0) return -1;

    uint8_t resp[FRAME_SIZE];
    int n = proto_read(dev->base.fd, resp, FRAME_SIZE);
    serial_flush_input(dev->base.fd);

    if (n != FRAME_SIZE) return -1;
    if (resp[0] != CMD_HELLO || resp[9] != CMD_HELLO) return -1;
    if (resp[1] != 'H' || resp[2] != 'E' || resp[3] != 'L' ||
        resp[4] != 'L' || resp[5] != 'O') return -1;
    if (resp[6] != 0x0A) return -1;

    switch (resp[7]) {
        case 0x01: dev->sub_revision = XF_SUB_REV_A01; break;
        case 0x02: dev->sub_revision = XF_SUB_REV_A02; break;
        case 0x11: dev->sub_revision = XF_SUB_REV_A11; break;
        case 0x12: dev->sub_revision = XF_SUB_REV_A12; break;
        default:
            fprintf(stderr, "panel: unknown sub-revision 0x%02X\n", resp[7]);
            return -1;
    }
    return 0;
}

xf_device_t *panel_open(const char *port)
{
    int fd = serial_open(port);
    if (fd < 0) return NULL;

    if (serial_configure(fd) < 0) {
        close(fd);
        return NULL;
    }

    xf_device_t *dev = (xf_device_t *)malloc(sizeof(*dev));
    if (!dev) {
        close(fd);
        return NULL;
    }

    dev->base.fd             = fd;
    dev->sub_revision        = XF_SUB_REV_UNKNOWN;
    dev->base.orientation    = XF_ORIENT_PORTRAIT;
    dev->base.display_width  = DISPLAY_WIDTH;
    dev->base.display_height = DISPLAY_HEIGHT;

    if (hello(dev) < 0) {
        fprintf(stderr, "panel_open: HELLO handshake failed on %s\n", port);
        free(dev);
        close(fd);
        return NULL;
    }

    return dev;
}

bool panel_is_flagship(const xf_device_t *dev)
{
    return dev->sub_revision == XF_SUB_REV_A02 ||
           dev->sub_revision == XF_SUB_REV_A12;
}

bool panel_is_brightness_range(const xf_device_t *dev)
{
    return dev->sub_revision == XF_SUB_REV_A11 ||
           dev->sub_revision == XF_SUB_REV_A12;
}

int panel_set_orientation(xf_device_t *dev, xf_orientation_t orientation)
{
    dev->base.orientation = orientation;

    uint8_t payload[8] = {0};
    payload[0] = (orientation == XF_ORIENT_PORTRAIT ||
                  orientation == XF_ORIENT_REVERSE_PORTRAIT)
                 ? HW_ORIENT_PORTRAIT
                 : HW_ORIENT_LANDSCAPE;

    return proto_send_cmd(dev->base.fd, CMD_SET_ORIENT, payload);
}

int panel_set_brightness(xf_device_t *dev, int level)
{
    if (level < 0)   level = 0;
    if (level > 100) level = 100;

    uint8_t val;
    if (panel_is_brightness_range(dev)) {
        val = (uint8_t)((level * 255) / 100);
    } else {
        /* Binary mode: 1 = off, 0 = full brightness (inverted). */
        val = (level == 0) ? 1 : 0;
    }

    uint8_t payload[8] = {0};
    payload[0] = val;
    return proto_send_cmd(dev->base.fd, CMD_SET_BRIGHTNESS, payload);
}

int panel_set_led(xf_device_t *dev, xf_color_t color)
{
    if (!panel_is_flagship(dev)) return 0;

    uint8_t payload[8] = {0};
    payload[0] = color.r;
    payload[1] = color.g;
    payload[2] = color.b;
    return proto_send_cmd(dev->base.fd, CMD_SET_LIGHTING, payload);
}

int panel_clear(xf_device_t *dev)
{
    xf_orientation_t saved = dev->base.orientation;

    /* Portrait gives us the native display_width/height without coordinate mapping. */
    panel_set_orientation(dev, XF_ORIENT_PORTRAIT);

    size_t sz = (size_t)(dev->base.display_width * dev->base.display_height * 3);
    uint8_t *white = (uint8_t *)malloc(sz);
    if (!white) {
        panel_set_orientation(dev, saved);
        return -1;
    }
    memset(white, 0xFF, sz);

    int r = panel_display_bitmap(dev, 0, 0,
                                 dev->base.display_width, dev->base.display_height,
                                 white);
    free(white);
    panel_set_orientation(dev, saved);
    return r;
}

int panel_screen_off(xf_device_t *dev) { return panel_set_brightness(dev, 0); }
int panel_screen_on(xf_device_t *dev)  { return panel_set_brightness(dev, 25); }

int panel_display_bitmap(xf_device_t *dev,
                         int x, int y,
                         int width, int height,
                         const uint8_t *rgb888)
{
    if (!dev || !rgb888) return -1;
    if (width <= 0 || height <= 0) return -1;

    int disp_w = panel_base_effective_width(&dev->base);
    int disp_h = panel_base_effective_height(&dev->base);
    if (width  > disp_w) width  = disp_w;
    if (height > disp_h) height = disp_h;

    int x0, y0, x1, y1;
    const uint8_t *pixels = rgb888;
    uint8_t *rotated = NULL;

    if (dev->base.orientation == XF_ORIENT_PORTRAIT ||
        dev->base.orientation == XF_ORIENT_LANDSCAPE) {
        x0 = x;
        y0 = y;
        x1 = x + width  - 1;
        y1 = y + height - 1;
    } else {
        /* Reverse orientations flip the region and rotate pixels so the device
         * renders the image in the correct visual direction. */
        x0 = dev->base.display_width  - x - width;
        y0 = dev->base.display_height - y - height;
        x1 = dev->base.display_width  - x - 1;
        y1 = dev->base.display_height - y - 1;

        rotated = (uint8_t *)malloc((size_t)(width * height * 3));
        if (!rotated) return -1;
        image_rotate_180(rgb888, width, height, rotated);
        pixels = rotated;
    }

    uint8_t payload[8] = {
        (uint8_t)((x0 >> 8) & 0xFF), (uint8_t)(x0 & 0xFF),
        (uint8_t)((y0 >> 8) & 0xFF), (uint8_t)(y0 & 0xFF),
        (uint8_t)((x1 >> 8) & 0xFF), (uint8_t)(x1 & 0xFF),
        (uint8_t)((y1 >> 8) & 0xFF), (uint8_t)(y1 & 0xFF)
    };
    proto_send_cmd(dev->base.fd, CMD_DISPLAY_BITMAP, payload);

    int pixel_count = width * height;
    uint8_t *rgb565 = (uint8_t *)malloc((size_t)(pixel_count * 2));
    if (!rgb565) {
        free(rotated);
        return -1;
    }
    image_rgb888_to_rgb565be(pixels, pixel_count, rgb565);

    size_t offset = 0;
    size_t total  = (size_t)(pixel_count * 2);
    while (offset < total) {
        size_t chunk = CHUNK_SIZE;
        if (offset + chunk > total) chunk = total - offset;
        proto_send_raw(dev->base.fd, rgb565 + offset, chunk);
        offset += chunk;
    }

    free(rgb565);
    free(rotated);

    /* 50 ms cooldown prevents bitmap corruption on macOS. */
    usleep(COOLDOWN_US);
    return 0;
}

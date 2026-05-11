#include "panel.h"
#include "turing_device_internal.h"
#include "panel_common.h"
#include "turing_protocol.h"
#include "image.h"
#include "port_detect.h"
#include "serial.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

static int hello(xf_device_t *dev)
{
    if (turing_proto_send_hello(dev->base.fd) < 0) return -1;

    uint8_t resp[TURING_RESP_LEN];
    int n = turing_proto_read(dev->base.fd, XF_BYTE_BUF(resp, TURING_RESP_LEN));
    serial_flush_input(dev->base.fd);

    if (n == TURING_RESP_LEN &&
        resp[0] == 0x01 && resp[1] == 0x01 && resp[2] == 0x01 &&
        resp[3] == 0x01 && resp[4] == 0x01 && resp[5] == 0x01) {
        dev->variant             = TURING_VARIANT_USBMONITOR_3_5;
        dev->base.display_width  = 320;
        dev->base.display_height = 480;
    } else if (n == TURING_RESP_LEN &&
               resp[0] == 0x02 && resp[1] == 0x02 && resp[2] == 0x02 &&
               resp[3] == 0x02 && resp[4] == 0x02 && resp[5] == 0x02) {
        dev->variant             = TURING_VARIANT_USBMONITOR_5;
        dev->base.display_width  = 480;
        dev->base.display_height = 800;
    } else if (n == TURING_RESP_LEN &&
               resp[0] == 0x03 && resp[1] == 0x03 && resp[2] == 0x03 &&
               resp[3] == 0x03 && resp[4] == 0x03 && resp[5] == 0x03) {
        dev->variant             = TURING_VARIANT_USBMONITOR_7;
        dev->base.display_width  = 600;
        dev->base.display_height = 1024;
    } else {
        /* Official Turing Smart Screen 3.5" does not answer HELLO. */
        dev->variant             = TURING_VARIANT_TURING_3_5;
        dev->base.display_width  = 320;
        dev->base.display_height = 480;
    }

    return 0;
}

static void turing_panel_cleanup(xf_device_t *dev)
{
    free(dev->img_scratch);
    free(dev->img_rgb565);
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
    dev->base.cleanup        = turing_panel_cleanup;
    dev->variant             = TURING_VARIANT_UNKNOWN;
    dev->base.orientation    = XF_ORIENT_PORTRAIT;
    dev->base.display_width  = 320;
    dev->base.display_height = 480;
    dev->img_scratch         = NULL;
    dev->img_rgb565          = NULL;

    if (hello(dev) < 0) {
        fprintf(stderr, "panel_open: HELLO failed on %s\n", port);
        free(dev);
        close(fd);
        return NULL;
    }

    dev->img_scratch = (uint8_t *)malloc(
        (size_t)(dev->base.display_width * dev->base.display_height * 3));
    dev->img_rgb565  = (uint8_t *)malloc(
        (size_t)(dev->base.display_width * dev->base.display_height * 2));
    if (!dev->img_scratch || !dev->img_rgb565) {
        turing_panel_cleanup(dev);
        free(dev);
        close(fd);
        return NULL;
    }

    return dev;
}

bool panel_is_flagship(const xf_device_t *dev)
{
    (void)dev;
    return false;
}

bool panel_is_brightness_range(const xf_device_t *dev)
{
    (void)dev;
    return true;
}

int panel_set_orientation(xf_device_t *dev, xf_orientation_t orientation)
{
    dev->base.orientation = orientation;
    int w = panel_base_effective_width(&dev->base);
    int h = panel_base_effective_height(&dev->base);
    return turing_proto_send_orient(dev->base.fd, (int)orientation, w, h);
}

int panel_set_brightness(xf_device_t *dev, int level)
{
    if (level < 0)   level = 0;
    if (level > 100) level = 100;

    /* Turing scale: 0 = brightest, 255 = darkest. */
    int val = 255 - ((level * 255) / 100);
    return turing_proto_send_cmd(dev->base.fd, TURING_CMD_SET_BRIGHTNESS, val, 0, 0, 0);
}

int panel_set_led(xf_device_t *dev, xf_color_t color)
{
    /* Turing devices have no LED control. */
    (void)dev;
    (void)color;
    return 0;
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

    xf_bitmap_t bm = {.x = 0, .y = 0, .width = dev->base.display_width, .height = dev->base.display_height, .rgb888 = white};
    int r = panel_display_bitmap(dev, bm);
    free(white);
    panel_set_orientation(dev, saved);
    return r;
}

int panel_screen_off(xf_device_t *dev)
{
    return turing_proto_send_cmd(dev->base.fd, TURING_CMD_SCREEN_OFF, 0, 0, 0, 0);
}

int panel_screen_on(xf_device_t *dev)
{
    return turing_proto_send_cmd(dev->base.fd, TURING_CMD_SCREEN_ON, 0, 0, 0, 0);
}

int panel_display_bitmap(xf_device_t *dev, xf_bitmap_t bm)
{
    if (!dev || !bm.rgb888) return -1;
    if (bm.width <= 0 || bm.height <= 0) return -1;

    int disp_w = panel_base_effective_width(&dev->base);
    int disp_h = panel_base_effective_height(&dev->base);
    if (bm.width  > disp_w) bm.width  = disp_w;
    if (bm.height > disp_h) bm.height = disp_h;

    int x0, y0, x1, y1;
    const uint8_t *pixels = bm.rgb888;
    uint8_t *rotated = NULL;

    if (dev->base.orientation == XF_ORIENT_PORTRAIT ||
        dev->base.orientation == XF_ORIENT_LANDSCAPE) {
        x0 = bm.x;
        y0 = bm.y;
        x1 = bm.x + bm.width  - 1;
        y1 = bm.y + bm.height - 1;
    } else {
        /* Reverse orientations flip the region and rotate pixels so the device
         * renders the image in the correct visual direction. */
        x0 = dev->base.display_width  - bm.x - bm.width;
        y0 = dev->base.display_height - bm.y - bm.height;
        x1 = dev->base.display_width  - bm.x - 1;
        y1 = dev->base.display_height - bm.y - 1;

        rotated = dev->img_scratch;
        image_rotate_180(bm.rgb888, bm.width, bm.height, rotated);
        pixels = rotated;
    }

    turing_proto_send_cmd(dev->base.fd, TURING_CMD_DISPLAY_BITMAP, x0, y0, x1, y1);

    int pixel_count = bm.width * bm.height;
    uint8_t *rgb565 = dev->img_rgb565;
    image_rgb888_to_rgb565le(pixels, pixel_count, rgb565);

    size_t offset = 0;
    size_t total  = (size_t)(pixel_count * 2);
    while (offset < total) {
        size_t chunk = TURING_CHUNK_SIZE;
        if (offset + chunk > total) chunk = total - offset;
        turing_proto_send_raw(dev->base.fd, XF_BYTE_SLICE(rgb565 + offset, chunk));
        offset += chunk;
    }

    /* 50 ms cooldown prevents bitmap corruption on macOS. */
    usleep(TURING_COOLDOWN_US);
    return 0;
}

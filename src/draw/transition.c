#include "transition.h"

#include <stdlib.h>
#include <string.h>

#define WIPE_STEPS 4

/*
 * Send a sub-rectangle of frame to the device.
 * When x==0 and w==fw the rows are contiguous in memory so the frame pointer
 * is passed directly without copying.
 */
static void blit(xf_device_t *dev, const uint8_t *frame, int fw,
                 int x, int y, int w, int h)
{
    if (!dev)
        return;

    if (x == 0 && w == fw) {
        panel_display_bitmap(dev, x, y, w, h, frame + (size_t)y * fw * 3);
        return;
    }

    uint8_t *sub = malloc((size_t)(w * h * 3));
    if (!sub)
        return;
    for (int row = 0; row < h; row++) {
        const uint8_t *src = frame + ((size_t)(y + row) * fw + (size_t)x) * 3;
        memcpy(sub + (size_t)(row * w * 3), src, (size_t)(w * 3));
    }
    panel_display_bitmap(dev, x, y, w, h, sub);
    free(sub);
}

static void trans_wipe_top(xf_device_t *dev, const uint8_t *old_f,
                            const uint8_t *new_f, int w, int h)
{
    (void)old_f;
    int band_h = h / WIPE_STEPS;
    for (int s = 0; s < WIPE_STEPS; s++) {
        int y  = s * band_h;
        int bh = (s == WIPE_STEPS - 1) ? h - y : band_h;
        blit(dev, new_f, w, 0, y, w, bh);
    }
}

static void trans_wipe_bottom(xf_device_t *dev, const uint8_t *old_f,
                               const uint8_t *new_f, int w, int h)
{
    (void)old_f;
    int band_h = h / WIPE_STEPS;
    for (int s = WIPE_STEPS - 1; s >= 0; s--) {
        int y  = s * band_h;
        int bh = (s == WIPE_STEPS - 1) ? h - y : band_h;
        blit(dev, new_f, w, 0, y, w, bh);
    }
}

static void trans_wipe_left(xf_device_t *dev, const uint8_t *old_f,
                             const uint8_t *new_f, int w, int h)
{
    (void)old_f;
    int band_w = w / WIPE_STEPS;
    for (int s = 0; s < WIPE_STEPS; s++) {
        int x  = s * band_w;
        int bw = (s == WIPE_STEPS - 1) ? w - x : band_w;
        blit(dev, new_f, w, x, 0, bw, h);
    }
}

static void trans_wipe_right(xf_device_t *dev, const uint8_t *old_f,
                              const uint8_t *new_f, int w, int h)
{
    (void)old_f;
    int band_w = w / WIPE_STEPS;
    for (int s = WIPE_STEPS - 1; s >= 0; s--) {
        int x  = s * band_w;
        int bw = (s == WIPE_STEPS - 1) ? w - x : band_w;
        blit(dev, new_f, w, x, 0, bw, h);
    }
}

/*
 * Build a frame where each pixel is taken from src_a or src_b depending on
 * which tile it falls in. tile_w and tile_h divide the screen into a grid;
 * tiles where (tx + ty) is even use src_a, odd tiles use src_b.
 */
static uint8_t *make_checker(const uint8_t *src_a, const uint8_t *src_b,
                              int w, int h, int tile_w, int tile_h)
{
    uint8_t *out = malloc((size_t)(w * h * 3));
    if (!out)
        return NULL;
    for (int y = 0; y < h; y++) {
        int ty = y / tile_h;
        for (int x = 0; x < w; x++) {
            int tx  = x / tile_w;
            int off = (y * w + x) * 3;
            const uint8_t *src = ((tx + ty) % 2 == 0) ? src_a : src_b;
            out[off + 0] = src[off + 0];
            out[off + 1] = src[off + 1];
            out[off + 2] = src[off + 2];
        }
    }
    return out;
}

static void trans_chess(xf_device_t *dev, const uint8_t *old_f,
                        const uint8_t *new_f, int w, int h)
{
    uint8_t *comp = make_checker(new_f, old_f, w, h, 40, 40);
    if (comp) {
        if (dev) panel_display_bitmap(dev, 0, 0, w, h, comp);
        free(comp);
    }
    if (dev) panel_display_bitmap(dev, 0, 0, w, h, new_f);
}

static void trans_strips_h(xf_device_t *dev, const uint8_t *old_f,
                            const uint8_t *new_f, int w, int h)
{
    uint8_t *comp = make_checker(new_f, old_f, w, h, w, 40);
    if (comp) {
        if (dev) panel_display_bitmap(dev, 0, 0, w, h, comp);
        free(comp);
    }
    if (dev) panel_display_bitmap(dev, 0, 0, w, h, new_f);
}

static void trans_strips_v(xf_device_t *dev, const uint8_t *old_f,
                            const uint8_t *new_f, int w, int h)
{
    uint8_t *comp = make_checker(new_f, old_f, w, h, 60, h);
    if (comp) {
        if (dev) panel_display_bitmap(dev, 0, 0, w, h, comp);
        free(comp);
    }
    if (dev) panel_display_bitmap(dev, 0, 0, w, h, new_f);
}

typedef void (*effect_fn_t)(xf_device_t *, const uint8_t *, const uint8_t *,
                            int, int);

static const effect_fn_t effects[XF_TRANS_COUNT] = {
    trans_wipe_top,
    trans_wipe_bottom,
    trans_wipe_left,
    trans_wipe_right,
    trans_chess,
    trans_strips_h,
    trans_strips_v,
};

void transition_play(xf_device_t *dev,
                     const uint8_t *old_frame,
                     const uint8_t *new_frame,
                     int width, int height,
                     xf_transition_t effect)
{
    if (effect < 0 || effect >= XF_TRANS_COUNT)
        return;
    effects[effect](dev, old_frame, new_frame, width, height);
}

#pragma once

#include "panel.h"
#include <stdint.h>

/*
 * Page-change transition effects.
 *
 * All effects are designed for a slow USB display:
 * - Wipe effects send 4 bands (1/4 of the screen each), no more.
 * - Composite effects (chess, strips) send exactly 2 full frames.
 * - No scrolling or slide effects: those require a full-frame send per pixel
 *   offset, which would stall the display for several seconds.
 *
 * To add a new effect: add a value before XF_TRANS_COUNT and implement the
 * corresponding static function in transition.c. The dispatch table picks it
 * up automatically.
 */
typedef enum {
    XF_TRANS_WIPE_TOP    = 0,
    XF_TRANS_WIPE_BOTTOM,
    XF_TRANS_WIPE_LEFT,
    XF_TRANS_WIPE_RIGHT,
    XF_TRANS_CHESS,
    XF_TRANS_STRIPS_H,
    XF_TRANS_STRIPS_V,
    XF_TRANS_COUNT
} xf_transition_t;

/*
 * Play a page-change transition from old_frame to new_frame.
 * After the call the device displays new_frame.
 * Passing dev=NULL runs headlessly (compositing logic still executes).
 */
void transition_play(xf_device_t *dev,
                     const uint8_t *old_frame,
                     const uint8_t *new_frame,
                     int width, int height,
                     xf_transition_t effect);

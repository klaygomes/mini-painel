#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "panel.h"

int main(void)
{
    /* Auto-detect the XuanFang device */
    xf_device_t *dev = panel_open_auto();
    if (!dev) {
        fprintf(stderr, "No XuanFang device found. "
                        "Is it plugged in?\n");
        return 1;
    }

    printf("Device opened — flagship: %s, brightness range: %s\n",
           panel_is_flagship(dev)         ? "yes" : "no",
           panel_is_brightness_range(dev) ? "yes" : "no");

    /* Set portrait orientation at 50% brightness */
    panel_set_orientation(dev, XF_ORIENT_PORTRAIT);
    panel_set_brightness(dev, 50);

    /* Set backplate LED to blue (no-op on non-flagship) */
    xf_color_t blue = {0, 0, 255};
    panel_set_led(dev, blue);

    /* Fill the screen with a solid red frame */
    int w = 320, h = 480;
    uint8_t *frame = (uint8_t *)malloc((size_t)(w * h * 3));
    if (!frame) {
        panel_close(dev);
        return 1;
    }
    memset(frame, 0, (size_t)(w * h * 3));

    int i;
    for (i = 0; i < w * h; i++) {
        frame[i * 3 + 0] = 0xFF; /* R */
        frame[i * 3 + 1] = 0x00; /* G */
        frame[i * 3 + 2] = 0x00; /* B */
    }

    printf("Displaying red frame...\n");
    if (panel_display_bitmap(dev, 0, 0, w, h, frame) < 0) {
        fprintf(stderr, "panel_display_bitmap failed\n");
    } else {
        printf("Done.\n");
    }

    free(frame);
    panel_close(dev);
    return 0;
}

#include "panel_common.h"
#include "port_detect.h"
#include <unistd.h>
#include <stdlib.h>

int panel_base_effective_width(const xf_device_base_t *base)
{
    return (base->orientation == XF_ORIENT_PORTRAIT ||
            base->orientation == XF_ORIENT_REVERSE_PORTRAIT)
           ? base->display_width : base->display_height;
}

int panel_base_effective_height(const xf_device_base_t *base)
{
    return (base->orientation == XF_ORIENT_PORTRAIT ||
            base->orientation == XF_ORIENT_REVERSE_PORTRAIT)
           ? base->display_height : base->display_width;
}

void panel_close(xf_device_t *dev)
{
    if (!dev) return;
    close(((xf_device_base_t *)dev)->fd);
    free(dev);
}

xf_device_t *panel_open_auto(void)
{
    char port[256];
    if (port_detect_auto(port, sizeof(port)) < 0) return NULL;
    return panel_open(port);
}

#include "comp_base.h"

void comp_fetch(xf_component_t *comp)
{
    if (comp->fetch)
        comp->fetch(comp);
}

void comp_render(xf_component_t *comp, uint8_t *buf, int w, int h)
{
    comp->render(comp, buf, w, h);
}

#include "comp_base.h"
#include "debug.h"

int comp_fetch(xf_component_t *comp)
{
    if (!comp)
        return XF_RES_ERR_INVALID_ARG;

    if (comp->fetch) {
        void *before = comp->ctx;
        int rc = comp->fetch(comp);
        DEBUG_LOG("comp=%p fetch=%p ctx_before=%p ctx_after=%p rc=%d",
                  (void *)comp, (void *)comp->fetch, before, comp->ctx, rc);
        (void)before;
        return rc;
    }

    DEBUG_LOG("comp=%p fetch=NULL ctx=%p", (void *)comp, comp->ctx);
    return XF_RES_OK;
}

int comp_render(xf_component_t *comp, uint8_t *buf, int w, int h)
{
    if (!comp || !comp->render || !buf || w <= 0 || h <= 0)
        return XF_RES_ERR_INVALID_ARG;

    DEBUG_LOG("comp=%p render=%p ctx=%p w=%d h=%d",
              (void *)comp, (void *)comp->render, comp->ctx, w, h);
    return comp->render(comp, buf, w, h);
}

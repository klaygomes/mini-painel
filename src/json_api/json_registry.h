#ifndef XF_JSON_REGISTRY_H
#define XF_JSON_REGISTRY_H

#include "json_kind.h"
#include "comp_base.h"

#define XF_JSON_REG_CAP 64

typedef struct {
    char                   id[64];
    const xf_kind_def_t   *def;
    xf_component_t        *comp;  /* heap-allocated; dashboard borrows ptr */
    void                  *data;  /* heap-allocated, NULL for stateless    */
} xf_reg_entry_t;

typedef struct {
    xf_reg_entry_t entries[XF_JSON_REG_CAP];
    int            count;
} xf_registry_t;

xf_reg_entry_t      *xf_registry_find  (xf_registry_t *r, const char *id);
xf_reg_entry_t      *xf_registry_add   (xf_registry_t *r, const char *id);
int                  xf_registry_remove(xf_registry_t *r, const char *id);

const xf_kind_def_t *xf_registry_kind_from_id(const char *id);

#endif

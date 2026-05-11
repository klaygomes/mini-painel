#include <string.h>
#include "json_registry.h"

xf_reg_entry_t *xf_registry_find(xf_registry_t *r, const char *id)
{
    int i;
    if (!r || !id)
        return NULL;
    for (i = 0; i < r->count; i++) {
        if (strcmp(r->entries[i].id, id) == 0)
            return &r->entries[i];
    }
    return NULL;
}

xf_reg_entry_t *xf_registry_add(xf_registry_t *r, const char *id)
{
    xf_reg_entry_t *e;
    if (!r || !id)
        return NULL;
    if (r->count >= XF_JSON_REG_CAP)
        return NULL;
    if (xf_registry_find(r, id))
        return NULL; /* duplicate */
    e = &r->entries[r->count++];
    memset(e, 0, sizeof(*e));
    strncpy(e->id, id, sizeof(e->id) - 1);
    e->id[sizeof(e->id) - 1] = '\0';
    return e;
}

int xf_registry_remove(xf_registry_t *r, const char *id)
{
    int i;
    if (!r || !id)
        return -1;
    for (i = 0; i < r->count; i++) {
        if (strcmp(r->entries[i].id, id) == 0) {
            if (i < r->count - 1)
                r->entries[i] = r->entries[r->count - 1];
            r->count--;
            return 0;
        }
    }
    return -1;
}

const xf_kind_def_t *xf_registry_kind_from_id(const char *id)
{
    const char *p;
    const char *kind_start;
    const char *kind_end;
    int         kind_len;

    if (!id)
        return NULL;

    /* Require "component." prefix */
    if (strncmp(id, "component.", 10) != 0)
        return NULL;

    kind_start = id + 10;
    p          = kind_start;
    while (*p && *p != '.')
        p++;

    if (*p != '.')
        return NULL; /* no trailing dot */

    kind_end = p;
    kind_len = (int)(kind_end - kind_start);
    if (kind_len <= 0)
        return NULL;

    return xf_kind_lookup((xf_str_slice_t){kind_start, kind_len});
}

#include "json_kind.h"
#include "json_decode_util.h"
#include "comp_checklist.h"

static int decode_checklist_item(xf_str_slice_t s, void *elem, int patch)
{
    comp_checklist_item_t *it = elem;
    if (xf_decode_str (s, "$.item", it->item, sizeof it->item, patch) < 0) return -1;
    if (xf_decode_bool(s, "$.done", &it->done, patch) < 0) return -1;
    return 0;
}

xf_component_t create_checklist(void *d) { return comp_checklist_create(d); }

int decode_checklist(xf_str_slice_t s, void *vdata, int patch)
{
    comp_checklist_data_t *d = vdata;
    if (xf_decode_str         (s, "$.title", d->title, sizeof d->title, patch) < 0) return -1;
    if (xf_decode_object_array(s, "$.items",
                               d->items, sizeof d->items[0],
                               COMP_CHECKLIST_MAX_ITEMS, &d->count,
                               decode_checklist_item, patch) < 0) return -1;
    return 0;
}

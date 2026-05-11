#include "json_kind.h"
#include "json_decode_util.h"
#include "comp_oncall.h"

xf_component_t create_oncall(void *d) { return comp_oncall_create(d); }

int decode_oncall(xf_str_slice_t s, void *vdata, int patch)
{
    comp_oncall_data_t *d = vdata;
    if (xf_decode_str  (s, "$.initials",    XF_STR_BUF(d->initials, sizeof d->initials), patch) < 0) return -1;
    if (xf_decode_str  (s, "$.name",        XF_STR_BUF(d->name, sizeof d->name), patch) < 0) return -1;
    if (xf_decode_str  (s, "$.role",        XF_STR_BUF(d->role, sizeof d->role), patch) < 0) return -1;
    if (xf_decode_str  (s, "$.phone",       XF_STR_BUF(d->phone, sizeof d->phone), patch) < 0) return -1;
    if (xf_decode_color(s, "$.avatar_color",&d->avatar_color, patch) < 0) return -1;
    return 0;
}

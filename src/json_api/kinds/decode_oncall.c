#include "json_kind.h"
#include "json_decode_util.h"
#include "comp_oncall.h"

xf_component_t create_oncall(void *d) { return comp_oncall_create(d); }

int decode_oncall(const char *s, int len, void *vdata, int patch)
{
    comp_oncall_data_t *d = vdata;
    if (xf_decode_str  (s, len, "$.initials",    d->initials,    sizeof d->initials,    patch) < 0) return -1;
    if (xf_decode_str  (s, len, "$.name",        d->name,        sizeof d->name,        patch) < 0) return -1;
    if (xf_decode_str  (s, len, "$.role",        d->role,        sizeof d->role,        patch) < 0) return -1;
    if (xf_decode_str  (s, len, "$.phone",       d->phone,       sizeof d->phone,       patch) < 0) return -1;
    if (xf_decode_color(s, len, "$.avatar_color",&d->avatar_color, patch) < 0) return -1;
    return 0;
}

#include "json_kind.h"
#include "json_decode_util.h"
#include "comp_build_status.h"

xf_component_t create_build_status(void *d) { return comp_build_status_create(d); }

int decode_build_status(xf_str_slice_t s, void *vdata, int patch)
{
    comp_build_status_data_t *d = vdata;
    if (xf_decode_str  (s, "$.branch",       XF_STR_BUF(d->branch, sizeof d->branch), patch) < 0) return -1;
    if (xf_decode_str  (s, "$.build_id",     XF_STR_BUF(d->build_id, sizeof d->build_id), patch) < 0) return -1;
    if (xf_decode_str  (s, "$.duration",     XF_STR_BUF(d->duration, sizeof d->duration), patch) < 0) return -1;
    if (xf_decode_str  (s, "$.status",       XF_STR_BUF(d->status, sizeof d->status), patch) < 0) return -1;
    if (xf_decode_color(s, "$.status_color", &d->status_color, patch) < 0) return -1;
    if (xf_decode_color(s, "$.status_fg",    &d->status_fg,   patch) < 0) return -1;
    return 0;
}

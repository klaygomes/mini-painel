#include "json_kind.h"
#include "json_decode_util.h"
#include "comp_sprint.h"

xf_component_t create_sprint(void *d) { return comp_sprint_create(d); }

int decode_sprint(xf_str_slice_t s, void *vdata, int patch)
{
    comp_sprint_data_t *d = vdata;
    if (xf_decode_str  (s, "$.title",          XF_STR_BUF(d->title, sizeof d->title), patch) < 0) return -1;
    if (xf_decode_str  (s, "$.progress_label", XF_STR_BUF(d->progress_label, sizeof d->progress_label), patch) < 0) return -1;
    if (xf_decode_str  (s, "$.time_left",      XF_STR_BUF(d->time_left, sizeof d->time_left), patch) < 0) return -1;
    if (xf_decode_float(s, "$.percent",        &d->percent,       patch) < 0) return -1;
    return 0;
}

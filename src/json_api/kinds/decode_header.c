#include "json_kind.h"
#include "json_decode_util.h"
#include "comp_header.h"

xf_component_t create_header(void *d) { return comp_header_create(d); }

int decode_header(xf_str_slice_t s, void *vdata, int patch)
{
    comp_header_data_t *d = vdata;
    if (xf_decode_str  (s, "$.date",        XF_STR_BUF(d->date, sizeof d->date), patch) < 0) return -1;
    if (xf_decode_str  (s, "$.status_text", XF_STR_BUF(d->status_text, sizeof d->status_text), patch) < 0) return -1;
    if (xf_decode_color(s, "$.status_dot",  &d->status_dot, patch) < 0) return -1;
    return 0;
}

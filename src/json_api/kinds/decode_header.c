#include "json_kind.h"
#include "json_decode_util.h"
#include "comp_header.h"

xf_component_t create_header(void *d) { return comp_header_create(d); }

int decode_header(const char *s, int len, void *vdata, int patch)
{
    comp_header_data_t *d = vdata;
    if (xf_decode_str  (s, len, "$.date",        d->date,        sizeof d->date,        patch) < 0) return -1;
    if (xf_decode_str  (s, len, "$.status_text", d->status_text, sizeof d->status_text, patch) < 0) return -1;
    if (xf_decode_color(s, len, "$.status_dot",  &d->status_dot, patch) < 0) return -1;
    return 0;
}

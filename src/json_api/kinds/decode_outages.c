#include "json_kind.h"
#include "json_decode_util.h"
#include "comp_outages.h"

static int decode_outage_row(xf_str_slice_t s, void *elem, int patch)
{
    comp_outage_row_t *r = elem;
    if (xf_decode_str  (s, "$.service",  XF_STR_BUF(r->service, sizeof r->service), patch) < 0) return -1;
    if (xf_decode_str  (s, "$.duration", XF_STR_BUF(r->duration, sizeof r->duration), patch) < 0) return -1;
    if (xf_decode_str  (s, "$.status",   XF_STR_BUF(r->status, sizeof r->status), patch) < 0) return -1;
    if (xf_decode_color(s, "$.row_bg",   &r->row_bg,  patch) < 0) return -1;
    if (xf_decode_color(s, "$.pill_bg",  &r->pill_bg, patch) < 0) return -1;
    if (xf_decode_color(s, "$.pill_fg",  &r->pill_fg, patch) < 0) return -1;
    if (xf_decode_color(s, "$.title_fg", &r->title_fg, patch) < 0) return -1;
    if (xf_decode_color(s, "$.dot",      &r->dot,     patch) < 0) return -1;
    return 0;
}

xf_component_t create_outages(void *d) { return comp_outages_create(d); }

int decode_outages(xf_str_slice_t s, void *vdata, int patch)
{
    comp_outages_data_t *d = vdata;
    if (xf_decode_str         (s, "$.title", XF_STR_BUF(d->title, sizeof d->title), patch) < 0) return -1;
    if (xf_decode_object_array(s, "$.rows",
                               d->rows, sizeof d->rows[0],
                               COMP_OUTAGES_MAX_ROWS, &d->count,
                               decode_outage_row, patch) < 0) return -1;
    return 0;
}

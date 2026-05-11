#include "json_kind.h"
#include "json_decode_util.h"
#include "comp_alerts.h"

static int decode_alert_row(xf_str_slice_t s, void *elem, int patch)
{
    comp_alert_row_t *r = elem;
    if (xf_decode_str  (s, "$.message", XF_STR_BUF(r->message, sizeof r->message), patch) < 0) return -1;
    if (xf_decode_str  (s, "$.time",    XF_STR_BUF(r->time, sizeof r->time), patch) < 0) return -1;
    if (xf_decode_color(s, "$.dot",     &r->dot,    patch) < 0) return -1;
    if (xf_decode_color(s, "$.row_bg",  &r->row_bg, patch) < 0) return -1;
    return 0;
}

xf_component_t create_alerts(void *d) { return comp_alerts_create(d); }

int decode_alerts(xf_str_slice_t s, void *vdata, int patch)
{
    comp_alerts_data_t *d = vdata;
    if (xf_decode_str         (s, "$.title", XF_STR_BUF(d->title, sizeof d->title), patch) < 0) return -1;
    if (xf_decode_object_array(s, "$.rows",
                               d->rows, sizeof d->rows[0],
                               COMP_ALERTS_MAX_ROWS, &d->count,
                               decode_alert_row, patch) < 0) return -1;
    return 0;
}

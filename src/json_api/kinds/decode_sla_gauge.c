#include "json_kind.h"
#include "json_decode_util.h"
#include "comp_sla_gauge.h"

static int decode_sla_row(const char *s, int len, void *elem, int patch)
{
    comp_sla_row_t *r = elem;
    if (xf_decode_str  (s, len, "$.label",   r->label,   sizeof r->label,   patch) < 0) return -1;
    if (xf_decode_str  (s, len, "$.value",   r->value,   sizeof r->value,   patch) < 0) return -1;
    if (xf_decode_float(s, len, "$.percent", &r->percent, patch) < 0) return -1;
    if (xf_decode_color(s, len, "$.bar",     &r->bar,    patch) < 0) return -1;
    return 0;
}

xf_component_t create_sla_gauge(void *d) { return comp_sla_gauge_create(d); }

int decode_sla_gauge(const char *s, int len, void *vdata, int patch)
{
    comp_sla_gauge_data_t *d = vdata;
    if (xf_decode_str         (s, len, "$.title", d->title, sizeof d->title, patch) < 0) return -1;
    if (xf_decode_object_array(s, len, "$.rows",
                               d->rows, sizeof d->rows[0],
                               COMP_SLA_GAUGE_MAX_ROWS, &d->count,
                               decode_sla_row, patch) < 0) return -1;
    return 0;
}

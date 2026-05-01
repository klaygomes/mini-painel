#include "json_kind.h"
#include "json_decode_util.h"
#include "comp_schedule.h"

static int decode_schedule_row(const char *s, int len, void *elem, int patch)
{
    comp_schedule_row_t *r = elem;
    if (xf_decode_str  (s, len, "$.time",  r->time,  sizeof r->time,  patch) < 0) return -1;
    if (xf_decode_str  (s, len, "$.event", r->event, sizeof r->event, patch) < 0) return -1;
    if (xf_decode_color(s, len, "$.bar",   &r->bar,  patch) < 0) return -1;
    return 0;
}

xf_component_t create_schedule(void *d) { return comp_schedule_create(d); }

int decode_schedule(const char *s, int len, void *vdata, int patch)
{
    comp_schedule_data_t *d = vdata;
    if (xf_decode_str         (s, len, "$.title", d->title, sizeof d->title, patch) < 0) return -1;
    if (xf_decode_object_array(s, len, "$.rows",
                               d->rows, sizeof d->rows[0],
                               COMP_SCHEDULE_MAX_ROWS, &d->count,
                               decode_schedule_row, patch) < 0) return -1;
    return 0;
}

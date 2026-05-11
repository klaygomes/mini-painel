#include "json_kind.h"
#include "json_decode_util.h"
#include "comp_sparkline.h"

xf_component_t create_sparkline(void *d) { return comp_sparkline_create(d); }

int decode_sparkline(xf_str_slice_t s, void *vdata, int patch)
{
    comp_sparkline_data_t *d = vdata;
    int i;
    float max_val;

    if (xf_decode_str        (s, "$.title",  XF_STR_BUF(d->title, sizeof d->title), patch) < 0) return -1;
    if (xf_decode_str        (s, "$.value",  XF_STR_BUF(d->value, sizeof d->value), patch) < 0) return -1;
    if (xf_decode_float_array(s, "$.points", d->points,
                              COMP_SPARKLINE_MAX_POINTS, &d->count, patch) < 0) return -1;

    /* Normalize to [0, 1] when raw values are outside that range.
     * Callers may send either pre-normalized fractions or raw-scale values
     * (e.g. percentages, counters). Dividing by the max preserves shape. */
    max_val = 0.0f;
    for (i = 0; i < d->count; i++)
        if (d->points[i] > max_val) max_val = d->points[i];
    if (max_val > 1.0f)
        for (i = 0; i < d->count; i++)
            d->points[i] /= max_val;

    return 0;
}

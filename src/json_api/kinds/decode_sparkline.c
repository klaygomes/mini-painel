#include "json_kind.h"
#include "json_decode_util.h"
#include "comp_sparkline.h"

xf_component_t create_sparkline(void *d) { return comp_sparkline_create(d); }

int decode_sparkline(const char *s, int len, void *vdata, int patch)
{
    comp_sparkline_data_t *d = vdata;
    if (xf_decode_str        (s, len, "$.title",  d->title,  sizeof d->title,  patch) < 0) return -1;
    if (xf_decode_str        (s, len, "$.value",  d->value,  sizeof d->value,  patch) < 0) return -1;
    if (xf_decode_float_array(s, len, "$.points", d->points,
                              COMP_SPARKLINE_MAX_POINTS, &d->count, patch) < 0) return -1;
    return 0;
}

#include "json_kind.h"
#include "json_decode_util.h"
#include "comp_error_rate.h"

xf_component_t create_error_rate(void *d) { return comp_error_rate_create(d); }

int decode_error_rate(xf_str_slice_t s, void *vdata, int patch)
{
    comp_error_rate_data_t *d = vdata;
    if (xf_decode_str        (s, "$.title", d->title, sizeof d->title, patch) < 0) return -1;
    if (xf_decode_str        (s, "$.value", d->value, sizeof d->value, patch) < 0) return -1;
    if (xf_decode_float_array(s, "$.bars",  d->bars,
                              COMP_ERROR_RATE_MAX_BARS, &d->count, patch) < 0) return -1;
    return 0;
}

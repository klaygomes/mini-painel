#include <stdio.h>
#include <string.h>
#include "json_kind.h"
#include "json_encode_util.h"
#include "comp_sla_gauge.h"
#include "mjson.h"

int encode_sla_gauge(const void *vdata, char *buf, size_t cap)
{
    const comp_sla_gauge_data_t *d = vdata;
    char tmp[256];
    int i, n;
    size_t pos = 0;

    n = mjson_snprintf(tmp, sizeof tmp,
        "{\"title\":%Q,\"count\":%d,\"rows\":[",
        d->title, d->count);
    if (n <= 0) return -1;
    XF_ENC_APPEND(tmp, n);

    for (i = 0; i < d->count; i++) {
        char bar[10];
        rgba_to_hex(d->rows[i].bar, bar);
        if (i > 0) XF_ENC_APPEND(",", 1);
        n = mjson_snprintf(tmp, sizeof tmp,
            "{\"label\":%Q,\"value\":%Q,\"percent\":%.6g,\"bar\":%Q}",
            d->rows[i].label, d->rows[i].value,
            (double)d->rows[i].percent, bar);
        if (n <= 0) return -1;
        XF_ENC_APPEND(tmp, n);
    }

    XF_ENC_APPEND("]}", 2);
    return 0;

}

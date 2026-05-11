#include <stdio.h>
#include <string.h>
#include "json_kind.h"
#include "json_enc_buf.h"
#include "comp_sparkline.h"
#include "mjson.h"

int encode_sparkline(const void *vdata, xf_out_buf_t out)
{
    const comp_sparkline_data_t *d = vdata;
    char tmp[64];
    int i, n;
    size_t pos = 0;

    n = mjson_snprintf(tmp, sizeof tmp,
        "{\"title\":%Q,\"value\":%Q,\"points\":[",
        d->title, d->value);
    if (n <= 0) return -1;
    XF_ENC_APPEND(tmp, n);

    for (i = 0; i < d->count; i++) {
        if (i > 0) XF_ENC_APPEND(",", 1);
        n = snprintf(tmp, sizeof tmp, "%.6g", (double)d->points[i]);
        if (n <= 0) return -1;
        XF_ENC_APPEND(tmp, n);
    }

    XF_ENC_APPEND("]}", 2);
    return 0;

}

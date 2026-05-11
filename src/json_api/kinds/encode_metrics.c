#include <stdio.h>
#include <string.h>
#include "json_kind.h"
#include "json_enc_buf.h"
#include "comp_metrics.h"
#include "mjson.h"

int encode_metrics(const void *vdata, xf_out_buf_t out)
{
    const comp_metrics_data_t *d = vdata;
    char  *buf = out.ptr;
    size_t cap = out.cap;
    char tmp[64];
    int i, n;
    size_t pos = 0;

    XF_ENC_APPEND("{\"count\":", 9);
    n = snprintf(tmp, sizeof tmp, "%d", d->count);
    if (n <= 0) return -1;
    XF_ENC_APPEND(tmp, n);
    XF_ENC_APPEND(",\"cards\":[", 10);

    for (i = 0; i < d->count; i++) {
        if (i > 0) XF_ENC_APPEND(",", 1);
        n = mjson_snprintf(tmp, sizeof tmp,
            "{\"label\":%Q,\"value\":%Q}",
            d->cards[i].label, d->cards[i].value);
        if (n <= 0) return -1;
        XF_ENC_APPEND(tmp, n);
    }

    XF_ENC_APPEND("]}", 2);
    return 0;

}

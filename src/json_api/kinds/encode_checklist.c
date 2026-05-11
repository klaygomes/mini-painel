#include <stdio.h>
#include <string.h>
#include "json_kind.h"
#include "json_enc_buf.h"
#include "comp_checklist.h"
#include "mjson.h"

int encode_checklist(const void *vdata, xf_out_buf_t out)
{
    const comp_checklist_data_t *d = vdata;
    char tmp[256];
    int i, n;
    size_t pos = 0;

    n = mjson_snprintf(tmp, sizeof tmp,
        "{\"title\":%Q,\"count\":%d,\"items\":[",
        d->title, d->count);
    if (n <= 0) return -1;
    XF_ENC_APPEND(tmp, n);

    for (i = 0; i < d->count; i++) {
        if (i > 0) XF_ENC_APPEND(",", 1);
        n = mjson_snprintf(tmp, sizeof tmp,
            "{\"item\":%Q,\"done\":%d}",
            d->items[i].item, d->items[i].done);
        if (n <= 0) return -1;
        XF_ENC_APPEND(tmp, n);
    }

    XF_ENC_APPEND("]}", 2);
    return 0;

}

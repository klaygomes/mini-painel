#include <stdio.h>
#include <string.h>
#include "json_kind.h"
#include "json_encode_util.h"
#include "comp_alerts.h"
#include "mjson.h"

int encode_alerts(const void *vdata, char *buf, size_t cap)
{
    const comp_alerts_data_t *d = vdata;
    char tmp[256];
    int i, n;
    size_t pos = 0;

    n = mjson_snprintf(tmp, sizeof tmp,
        "{\"title\":%Q,\"count\":%d,\"rows\":[",
        d->title, d->count);
    if (n <= 0) return -1;
    XF_ENC_APPEND(tmp, n);

    for (i = 0; i < d->count; i++) {
        char dot[10], row_bg[10];
        rgba_to_hex(d->rows[i].dot,    dot);
        rgba_to_hex(d->rows[i].row_bg, row_bg);
        if (i > 0) XF_ENC_APPEND(",", 1);
        n = mjson_snprintf(tmp, sizeof tmp,
            "{\"message\":%Q,\"time\":%Q,\"dot\":%Q,\"row_bg\":%Q}",
            d->rows[i].message, d->rows[i].time, dot, row_bg);
        if (n <= 0) return -1;
        XF_ENC_APPEND(tmp, n);
    }

    XF_ENC_APPEND("]}", 2);
    return 0;

}

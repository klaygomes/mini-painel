#include <stdio.h>
#include <string.h>
#include "json_kind.h"
#include "json_encode_util.h"
#include "comp_outages.h"
#include "mjson.h"

int encode_outages(const void *vdata, xf_out_buf_t out)
{
    const comp_outages_data_t *d = vdata;
    char tmp[512];
    int i, n;
    size_t pos = 0;

    n = mjson_snprintf(tmp, sizeof tmp,
        "{\"title\":%Q,\"count\":%d,\"rows\":[",
        d->title, d->count);
    if (n <= 0) return -1;
    XF_ENC_APPEND(tmp, n);

    for (i = 0; i < d->count; i++) {
        char row_bg[10], pill_bg[10], pill_fg[10], title_fg[10], dot[10];
        rgba_to_hex(d->rows[i].row_bg,   row_bg);
        rgba_to_hex(d->rows[i].pill_bg,  pill_bg);
        rgba_to_hex(d->rows[i].pill_fg,  pill_fg);
        rgba_to_hex(d->rows[i].title_fg, title_fg);
        rgba_to_hex(d->rows[i].dot,      dot);
        if (i > 0) XF_ENC_APPEND(",", 1);
        n = mjson_snprintf(tmp, sizeof tmp,
            "{\"service\":%Q,\"duration\":%Q,\"status\":%Q,"
            "\"row_bg\":%Q,\"pill_bg\":%Q,\"pill_fg\":%Q,"
            "\"title_fg\":%Q,\"dot\":%Q}",
            d->rows[i].service, d->rows[i].duration, d->rows[i].status,
            row_bg, pill_bg, pill_fg, title_fg, dot);
        if (n <= 0) return -1;
        XF_ENC_APPEND(tmp, n);
    }

    XF_ENC_APPEND("]}", 2);
    return 0;

}

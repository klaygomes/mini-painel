#include <stdio.h>
#include <string.h>
#include "json_kind.h"
#include "json_encode_util.h"
#include "comp_team_status.h"
#include "mjson.h"

int encode_team_status(const void *vdata, xf_str_buf_t out)
{
    const comp_team_status_data_t *d = vdata;
    char tmp[256];
    int i, n;
    size_t pos = 0;

    n = mjson_snprintf(tmp, sizeof tmp,
        "{\"title\":%Q,\"count\":%d,\"members\":[",
        d->title, d->count);
    if (n <= 0) return -1;
    XF_ENC_APPEND(tmp, n);

    for (i = 0; i < d->count; i++) {
        char ac[10];
        rgba_to_hex(d->members[i].avatar_color, ac);
        if (i > 0) XF_ENC_APPEND(",", 1);
        n = mjson_snprintf(tmp, sizeof tmp,
            "{\"initials\":%Q,\"name\":%Q,\"avatar_color\":%Q,\"online\":%d}",
            d->members[i].initials, d->members[i].name, ac, d->members[i].online);
        if (n <= 0) return -1;
        XF_ENC_APPEND(tmp, n);
    }

    XF_ENC_APPEND("]}", 2);
    return 0;

}

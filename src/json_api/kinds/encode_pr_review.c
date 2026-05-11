#include <stdio.h>
#include <string.h>
#include "json_kind.h"
#include "json_encode_util.h"
#include "comp_pr_review.h"
#include "mjson.h"

int encode_pr_review(const void *vdata, xf_out_buf_t out)
{
    const comp_pr_review_data_t *d = vdata;
    char tmp[256];
    int i, n;
    size_t pos = 0;

    n = mjson_snprintf(tmp, sizeof tmp,
        "{\"title\":%Q,\"count\":%d,\"rows\":[",
        d->title, d->count);
    if (n <= 0) return -1;
    XF_ENC_APPEND(tmp, n);

    for (i = 0; i < d->count; i++) {
        char ac[10];
        rgba_to_hex(d->rows[i].avatar_color, ac);
        if (i > 0) XF_ENC_APPEND(",", 1);
        n = mjson_snprintf(tmp, sizeof tmp,
            "{\"initials\":%Q,\"title\":%Q,\"age\":%Q,"
            "\"reviews\":%d,\"avatar_color\":%Q}",
            d->rows[i].initials, d->rows[i].title, d->rows[i].age,
            d->rows[i].reviews, ac);
        if (n <= 0) return -1;
        XF_ENC_APPEND(tmp, n);
    }

    XF_ENC_APPEND("]}", 2);
    return 0;

}

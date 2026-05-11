#include <stdio.h>
#include "json_kind.h"
#include "json_encode_util.h"
#include "comp_header.h"
#include "mjson.h"

int encode_header(const void *vdata, xf_str_buf_t out)
{
    const comp_header_data_t *d = vdata;
    char dot[10];
    rgba_to_hex(d->status_dot, dot);
    int n = mjson_snprintf(out.ptr, out.cap,
        "{\"date\":%Q,\"status_text\":%Q,\"status_dot\":%Q}",
        d->date, d->status_text, dot);
    return (n > 0 && (size_t)n < out.cap) ? 0 : -1;
}

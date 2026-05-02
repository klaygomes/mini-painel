#include "json_kind.h"
#include "json_encode_util.h"
#include "comp_build_status.h"
#include "mjson.h"

int encode_build_status(const void *vdata, char *buf, size_t cap)
{
    const comp_build_status_data_t *d = vdata;
    char sc[10], sf[10];
    rgba_to_hex(d->status_color, sc);
    rgba_to_hex(d->status_fg, sf);
    int n = mjson_snprintf(buf, cap,
        "{\"branch\":%Q,\"build_id\":%Q,\"duration\":%Q,"
        "\"status\":%Q,\"status_color\":%Q,\"status_fg\":%Q}",
        d->branch, d->build_id, d->duration,
        d->status, sc, sf);
    return (n > 0 && (size_t)n < cap) ? 0 : -1;
}

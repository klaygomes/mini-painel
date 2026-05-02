#include "json_kind.h"
#include "json_encode_util.h"
#include "comp_oncall.h"
#include "mjson.h"

int encode_oncall(const void *vdata, char *buf, size_t cap)
{
    const comp_oncall_data_t *d = vdata;
    char ac[10];
    rgba_to_hex(d->avatar_color, ac);
    int n = mjson_snprintf(buf, cap,
        "{\"initials\":%Q,\"name\":%Q,\"role\":%Q,\"phone\":%Q,\"avatar_color\":%Q}",
        d->initials, d->name, d->role, d->phone, ac);
    return (n > 0 && (size_t)n < cap) ? 0 : -1;
}

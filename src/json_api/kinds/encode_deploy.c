#include "json_kind.h"
#include "comp_deploy.h"
#include "mjson.h"

int encode_deploy(const void *vdata, char *buf, size_t cap)
{
    const comp_deploy_data_t *d = vdata;
    int n = mjson_snprintf(buf, cap,
        "{\"branch\":%Q,\"time_ago\":%Q,\"label\":%Q}",
        d->branch, d->time_ago, d->label);
    return (n > 0 && (size_t)n < cap) ? 0 : -1;
}

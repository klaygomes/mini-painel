#include "json_kind.h"
#include "json_decode_util.h"
#include "comp_deploy.h"

xf_component_t create_deploy(void *d) { return comp_deploy_create(d); }

int decode_deploy(const char *s, int len, void *vdata, int patch)
{
    comp_deploy_data_t *d = vdata;
    if (xf_decode_str(s, len, "$.branch",   d->branch,   sizeof d->branch,   patch) < 0) return -1;
    if (xf_decode_str(s, len, "$.time_ago", d->time_ago, sizeof d->time_ago, patch) < 0) return -1;
    if (xf_decode_str(s, len, "$.label",    d->label,    sizeof d->label,    patch) < 0) return -1;
    return 0;
}

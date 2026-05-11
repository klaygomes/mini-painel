#include "json_kind.h"
#include "json_decode_util.h"
#include "comp_deploy.h"

xf_component_t create_deploy(void *d) { return comp_deploy_create(d); }

int decode_deploy(xf_str_slice_t s, void *vdata, int patch)
{
    comp_deploy_data_t *d = vdata;
    if (xf_decode_str(s, "$.branch",   d->branch,   sizeof d->branch,   patch) < 0) return -1;
    if (xf_decode_str(s, "$.time_ago", d->time_ago, sizeof d->time_ago, patch) < 0) return -1;
    if (xf_decode_str(s, "$.label",    d->label,    sizeof d->label,    patch) < 0) return -1;
    int status = d->status;
    if (xf_decode_int(s, "$.status",   &status,                         patch) < 0) return -1;
    d->status = (comp_deploy_status_t)status;
    return 0;
}

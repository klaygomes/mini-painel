#include "json_kind.h"
#include "comp_sprint.h"
#include "mjson.h"

int encode_sprint(const void *vdata, char *buf, size_t cap)
{
    const comp_sprint_data_t *d = vdata;
    int n = mjson_snprintf(buf, cap,
        "{\"title\":%Q,\"progress_label\":%Q,\"time_left\":%Q,\"percent\":%.6g}",
        d->title, d->progress_label, d->time_left, (double)d->percent);
    return (n > 0 && (size_t)n < cap) ? 0 : -1;
}

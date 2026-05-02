#include "json_kind.h"

int encode_stateless(const void *vdata, char *buf, size_t cap)
{
    (void)vdata;
    if (cap < 3) return -1;
    buf[0] = '{'; buf[1] = '}'; buf[2] = '\0';
    return 0;
}

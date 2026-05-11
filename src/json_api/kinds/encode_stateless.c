#include "json_kind.h"

int encode_divider(const void *vdata, xf_str_buf_t out)
{
    (void)vdata;
    if (out.cap < 3) return -1;
    out.ptr[0] = '{'; out.ptr[1] = '}'; out.ptr[2] = '\0';
    return 0;
}

int encode_spacer(const void *vdata, xf_str_buf_t out)
{
    (void)vdata;
    if (out.cap < 3) return -1;
    out.ptr[0] = '{'; out.ptr[1] = '}'; out.ptr[2] = '\0';
    return 0;
}

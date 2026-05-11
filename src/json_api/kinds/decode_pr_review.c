#include "json_kind.h"
#include "json_decode_util.h"
#include "comp_pr_review.h"

static int decode_pr_row(xf_str_slice_t s, void *elem, int patch)
{
    comp_pr_row_t *r = elem;
    if (xf_decode_str  (s, "$.initials",    XF_STR_BUF(r->initials, sizeof r->initials), patch) < 0) return -1;
    if (xf_decode_str  (s, "$.title",       XF_STR_BUF(r->title, sizeof r->title), patch) < 0) return -1;
    if (xf_decode_str  (s, "$.age",         XF_STR_BUF(r->age, sizeof r->age), patch) < 0) return -1;
    if (xf_decode_int  (s, "$.reviews",     &r->reviews,    patch) < 0) return -1;
    if (xf_decode_color(s, "$.avatar_color",&r->avatar_color, patch) < 0) return -1;
    return 0;
}

xf_component_t create_pr_review(void *d) { return comp_pr_review_create(d); }

int decode_pr_review(xf_str_slice_t s, void *vdata, int patch)
{
    comp_pr_review_data_t *d = vdata;
    if (xf_decode_str         (s, "$.title", XF_STR_BUF(d->title, sizeof d->title), patch) < 0) return -1;
    if (xf_decode_object_array(s, "$.rows",
                               d->rows, sizeof d->rows[0],
                               COMP_PR_REVIEW_MAX_ROWS, &d->count,
                               decode_pr_row, patch) < 0) return -1;
    return 0;
}

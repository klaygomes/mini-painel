#include "json_kind.h"
#include "json_decode_util.h"
#include "comp_team_status.h"

static int decode_team_member(xf_str_slice_t s, void *elem, int patch)
{
    comp_team_member_t *m = elem;
    if (xf_decode_str  (s, "$.initials",    XF_STR_BUF(m->initials, sizeof m->initials), patch) < 0) return -1;
    if (xf_decode_str  (s, "$.name",        XF_STR_BUF(m->name, sizeof m->name), patch) < 0) return -1;
    if (xf_decode_color(s, "$.avatar_color",&m->avatar_color, patch) < 0) return -1;
    if (xf_decode_bool (s, "$.online",      &m->online,     patch) < 0) return -1;
    return 0;
}

xf_component_t create_team_status(void *d) { return comp_team_status_create(d); }

int decode_team_status(xf_str_slice_t s, void *vdata, int patch)
{
    comp_team_status_data_t *d = vdata;
    if (xf_decode_str         (s, "$.title", XF_STR_BUF(d->title, sizeof d->title), patch) < 0) return -1;
    if (xf_decode_object_array(s, "$.members",
                               d->members, sizeof d->members[0],
                               COMP_TEAM_STATUS_MAX_MEMBERS, &d->count,
                               decode_team_member, patch) < 0) return -1;
    return 0;
}

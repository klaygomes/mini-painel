#include <string.h>
#include "json_kind.h"

/* Pull in all component headers so sizeof() and COMP_*_HEIGHT resolve. */
#include "comp_header.h"
#include "comp_divider.h"
#include "comp_spacer.h"
#include "comp_deploy.h"
#include "comp_build_status.h"
#include "comp_metrics.h"
#include "comp_sparkline.h"
#include "comp_error_rate.h"
#include "comp_alerts.h"
#include "comp_outages.h"
#include "comp_sprint.h"
#include "comp_team_status.h"
#include "comp_oncall.h"
#include "comp_sla_gauge.h"
#include "comp_schedule.h"
#include "comp_pr_review.h"
#include "comp_checklist.h"

/* Forward-declare every create, decode, and encode function. */
#define XF_KIND(name, dsz, h) \
    extern xf_component_t create_##name(void *);                  \
    extern int decode_##name(const char *, int, void *, int);     \
    extern int encode_##name(const void *, char *, size_t);
#include "json_kinds.def"
#undef XF_KIND

static const xf_kind_def_t TABLE[] = {
#define XF_KIND(name, dsz, h) \
    { #name, (size_t)(dsz), (h), create_##name, decode_##name, encode_##name, (dsz) == 0 },
#include "json_kinds.def"
#undef XF_KIND
};

#define TABLE_COUNT ((int)(sizeof(TABLE) / sizeof(TABLE[0])))

const xf_kind_def_t *xf_kind_lookup(const char *name, int name_len)
{
    int i;
    if (!name || name_len <= 0)
        return NULL;
    for (i = 0; i < TABLE_COUNT; i++) {
        if ((int)strlen(TABLE[i].name) == name_len &&
            memcmp(TABLE[i].name, name, (size_t)name_len) == 0)
            return &TABLE[i];
    }
    return NULL;
}

int xf_kind_count(void)
{
    return TABLE_COUNT;
}

const xf_kind_def_t *xf_kind_at(int i)
{
    if (i < 0 || i >= TABLE_COUNT)
        return NULL;
    return &TABLE[i];
}

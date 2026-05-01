#include "json_kind.h"
#include "json_decode_util.h"
#include "comp_metrics.h"

static int decode_metric_card(const char *s, int len, void *elem, int patch)
{
    comp_metric_card_t *c = elem;
    if (xf_decode_str(s, len, "$.label", c->label, sizeof c->label, patch) < 0) return -1;
    if (xf_decode_str(s, len, "$.value", c->value, sizeof c->value, patch) < 0) return -1;
    return 0;
}

xf_component_t create_metrics(void *d) { return comp_metrics_create(d); }

int decode_metrics(const char *s, int len, void *vdata, int patch)
{
    comp_metrics_data_t *d = vdata;
    if (xf_decode_object_array(s, len, "$.cards",
                               d->cards, sizeof d->cards[0],
                               COMP_METRICS_MAX_CARDS, &d->count,
                               decode_metric_card, patch) < 0) return -1;
    return 0;
}

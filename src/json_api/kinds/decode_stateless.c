#include "json_kind.h"
#include "comp_divider.h"
#include "comp_spacer.h"

xf_component_t create_divider(void *d) { (void)d; return comp_divider_create(); }
xf_component_t create_spacer (void *d) { (void)d; return comp_spacer_create();  }

int decode_stateless(const char *s, int len, void *d, int patch)
{
    (void)s; (void)len; (void)d; (void)patch;
    return 0;
}

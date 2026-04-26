#include "theme.h"

static const xf_theme_t *g_theme = &xf_theme_default;

void xf_set_theme(const xf_theme_t *theme)
{
    g_theme = theme;
}

const xf_theme_t *xf_get_theme(void)
{
    return g_theme;
}

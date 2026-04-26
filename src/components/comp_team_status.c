/* White halo behind the presence dot ensures it reads clearly on any avatar
 * colour without requiring a separate border colour. */

#include "comp_team_status.h"
#include "comp_gfx.h"

#define HEADER_H LAY_HEADER_H
#define AVATAR_R    13.0
#define AVATAR_GAP   6.0
#define START_X     14.0

static void draw(xf_draw_ctx_t *ctx, void *user_data)
{
    const xf_theme_t             *t = xf_get_theme();
    const comp_team_status_data_t *d = user_data;

    xf_gfx_section_label(ctx, d->title, 13.0);

    double cy  = (double)HEADER_H + AVATAR_R + 2.0;
    double step = AVATAR_R * 2.0 + AVATAR_GAP;
    double x    = START_X;

    for (int i = 0; i < d->count; i++) {
        const comp_team_member_t *m = &d->members[i];

        xf_gfx_avatar(ctx, x, cy, AVATAR_R,
                      m->online ? m->avatar_color : t->offline, m->initials);

        if (m->online)
            xf_gfx_online_dot(ctx, x + AVATAR_R - 3.5, cy + AVATAR_R - 3.5, t->success);

        x += step;
    }
}

static void render(xf_component_t *self, uint8_t *buf, int w, int h)
{
    xf_render(buf, w, h, draw, self->ctx);
}

xf_component_t comp_team_status_create(comp_team_status_data_t *data)
{
    xf_component_t c = XF_COMPONENT_DATA(render, data);
    return c;
}

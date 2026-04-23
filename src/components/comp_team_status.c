/* White halo behind the presence dot ensures it reads clearly on any avatar
 * colour without requiring a separate border colour. */

#include "comp_team_status.h"
#include "draw.h"
#include "layout.h"

#define HEADER_H LAY_HEADER_H
#define AVATAR_R    13.0
#define AVATAR_GAP   6.0
#define START_X     14.0

static void draw(xf_draw_ctx_t *ctx, int w, int h, void *user_data)
{
    const xf_theme_t             *t = xf_get_theme();
    const comp_team_status_data_t *d = user_data;
    (void)w; (void)h;

    xf_draw_text(ctx, d->title, LAY_PAD_X, 13.0, &(xf_text_opts_t){
        .size = FONT_MD, .weight = 600, .color = t->text_muted
    });

    double cy  = (double)HEADER_H + AVATAR_R + 2.0;
    double step = AVATAR_R * 2.0 + AVATAR_GAP;
    double x    = START_X;

    for (int i = 0; i < d->count; i++) {
        const comp_team_member_t *m = &d->members[i];

        xf_draw_circle(ctx, x, cy, AVATAR_R,
                       m->online ? m->avatar_color : t->offline);

        xf_draw_text(ctx, m->initials, x, cy + 5.0, &(xf_text_opts_t){
            .size = FONT_SM, .weight = 700, .color = t->white, .align = XF_TEXT_CENTER
        });

        if (m->online) {
            /* White halo so the green dot contrasts against any avatar hue */
            xf_draw_circle(ctx, x + AVATAR_R - 3.5, cy + AVATAR_R - 3.5,
                           4.0, t->white);
            xf_draw_circle(ctx, x + AVATAR_R - 3.5, cy + AVATAR_R - 3.5,
                           2.8, t->success);
        }

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

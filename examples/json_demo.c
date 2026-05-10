#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "json_api.h"
#include "dashboard.h"
#include "comp_header.h"

#define DISPLAY_W 480
#define DISPLAY_H 320

static uint8_t canvas[DISPLAY_W * DISPLAY_H * 3];
static char    reply[4096];

static const char SCRIPT[] =
"["
" {\"op\":\"row.add\",\"id\":\"component.header.main\","
"  \"data\":{\"date\":\"Thu \\u00b7 Apr 23\",\"status_text\":\"all clear\",\"status_dot\":\"#1D9E75\"}},"
" {\"op\":\"page.render\"}"
"]";

static void write_ppm(const char *path, const uint8_t *fb, int w, int h)
{
    FILE *f = fopen(path, "wb");
    if (!f) {
        fprintf(stderr, "cannot open %s\n", path);
        return;
    }
    fprintf(f, "P6\n%d %d\n255\n", w, h);
    fwrite(fb, 1, (size_t)(w * h * 3), f);
    fclose(f);
}

int main(void)
{
    xf_json_ctx_t *c;
    int            rc;

    c = xf_json_create(DISPLAY_W, DISPLAY_H, 0);
    if (!c) {
        fprintf(stderr, "xf_json_create failed\n");
        return 1;
    }

    {
        const xf_json_exec_req_t req = {
            .ctx = c,
            .json = {
                .buf = SCRIPT,
                .size = strlen(SCRIPT),
            },
            .canvas = {
                .buf = canvas,
                .size = sizeof canvas,
            },
            .out_json = {
                .buf = reply,
                .size = sizeof reply,
            },
        };
        rc = xf_json_exec(&req);
    }
    fprintf(stderr, "rc=%d reply=%s\n", rc, reply);
    {
        int nw = 0;
        for (int i = 0; i < DISPLAY_W * DISPLAY_H * 3; i += 3)
            if (canvas[i] != 0xff || canvas[i+1] != 0xff || canvas[i+2] != 0xff) nw++;
        fprintf(stderr, "canvas non-white immediately after exec: %d\n", nw);
    }

    if (rc == 0)
        write_ppm("bin/json_demo.ppm", canvas, DISPLAY_W, DISPLAY_H);

    xf_json_destroy(c);

    /* direct render test to compare */
    {
        comp_header_data_t d = {0};
        snprintf(d.date, sizeof(d.date), "Thu Apr 23");
        snprintf(d.status_text, sizeof(d.status_text), "all clear");
        xf_component_t hcomp = comp_header_create(&d);
        xf_dashboard_t *dash2 = dashboard_create(DISPLAY_W, DISPLAY_H, 0);
        int rc2;
        const uint8_t *fb2 = NULL;
        dashboard_add_full_row(dash2, &hcomp, COMP_HEADER_HEIGHT);
        rc2 = dashboard_render(dash2, &fb2);
        if (rc2 < 0 || !fb2) {
            fprintf(stderr, "direct dashboard_render failed rc=%d\n", rc2);
            dashboard_destroy(dash2);
            return 1;
        }
        int nw = 0;
        for (int i = 0; i < DISPLAY_W * DISPLAY_H * 3; i += 3)
            if (fb2[i] != 0xff || fb2[i+1] != 0xff || fb2[i+2] != 0xff) nw++;
        fprintf(stderr, "direct render non-white: %d\n", nw);
        write_ppm("bin/json_demo_direct.ppm", fb2, DISPLAY_W, DISPLAY_H);
        dashboard_destroy(dash2);
    }

    return rc == 0 ? 0 : 1;
}

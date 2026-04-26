#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <time.h>

#include "panel.h"
#include "dashboard.h"
#include "transition.h"
#include "draw.h"
#include "components/comp_header.h"
#include "components/comp_divider.h"
#include "components/comp_spacer.h"
#include "components/comp_deploy.h"
#include "components/comp_build_status.h"
#include "components/comp_metrics.h"
#include "components/comp_sparkline.h"
#include "components/comp_error_rate.h"
#include "components/comp_alerts.h"
#include "components/comp_outages.h"
#include "components/comp_sprint.h"
#include "components/comp_team_status.h"
#include "components/comp_oncall.h"
#include "components/comp_sla_gauge.h"
#include "components/comp_schedule.h"
#include "components/comp_pr_review.h"
#include "components/comp_checklist.h"

#define DISPLAY_W  480
#define DISPLAY_H  320
#define PAGE_TICKS 30    /* 30 ticks × 2 s = 60 s per page */
#define UPDATE_US  2000000

static uint8_t old_frame_buf[DISPLAY_W * DISPLAY_H * 3];

static const xf_rgba_t COL_RED = XF_RGB(0xE24B4A);
static const xf_rgba_t COL_ORG = XF_RGB(0xEF9F27);
static const xf_rgba_t COL_GRN = XF_RGB(0x1D9E75);

typedef struct { xf_device_t *dev; const uint8_t *frame; } send_ctx_t;

static void send_rect(xf_device_t *dev, const uint8_t *frame,
                      int x, int y, int w, int h)
{
    int      bpp = 3;
    uint8_t *sub = malloc((size_t)(w * h * bpp));
    if (!sub)
        return;
    for (int row = 0; row < h; row++) {
        const uint8_t *src = frame + ((size_t)(y + row) * DISPLAY_W + (size_t)x) * bpp;
        memcpy(sub + (size_t)(row * w * bpp), src, (size_t)(w * bpp));
    }
    if (dev)
        panel_display_bitmap(dev, x, y, w, h, sub);
    free(sub);
}

static void on_dirty_rect(int x, int y, int w, int h, void *ctx)
{
    send_ctx_t *sc = ctx;
    send_rect(sc->dev, sc->frame, x, y, w, h);
}

static void fmt_ago(char *buf, size_t n, int s)
{
    int m = s / 60;
    if (m < 1) m = 1;
    if (m >= 60)
        snprintf(buf, n, "%dh %dm ago", m / 60, m % 60);
    else
        snprintf(buf, n, "%dm ago", m);
}

static void fmt_dur(char *buf, size_t n, int s)
{
    int m = s / 60;
    if (m < 1) m = 1;
    if (m >= 60)
        snprintf(buf, n, "%dh %dm", m / 60, m % 60);
    else
        snprintf(buf, n, "%dm", m);
}

static void fmt_pr_age(char *buf, size_t n, int h)
{
    if (h >= 24)
        snprintf(buf, n, "%dd", h / 24);
    else
        snprintf(buf, n, "%dh", h < 1 ? 1 : h);
}

int main(void)
{
    srand((unsigned)time(NULL));

    comp_header_data_t header = {
        .date        = "Thu \xc2\xb7 Apr 23",
        .status_text = "2 alerts",
        .status_dot  = XF_RGB(0xE24B4A),
    };

    comp_deploy_data_t deploy = {
        .branch   = "main@a3f9c1e",
        .time_ago = "8m",
        .label    = "prod green",
    };

    comp_build_status_data_t build = {
        .branch       = "main",
        .build_id     = "#1847",
        .duration     = "2m 34s",
        .status       = "passing",
        .status_color = XF_RGB(0x1D9E75),
        .status_fg    = XF_RGB(0xFFFFFF),
    };

    comp_metrics_data_t metrics = {
        .cards = {
            { "UPTIME", "99.94%" },
            { "P99",    "142ms"  },
            { "RPS",    "412"    },
            { "ERRORS", "0.3%"   },
        },
        .count = 4,
    };

    comp_sparkline_data_t sparkline = {
        .title  = "req/s",
        .value  = "412",
        .points = {
            0.30f, 0.40f, 0.35f, 0.50f, 0.60f, 0.55f, 0.70f, 0.65f,
            0.80f, 0.75f, 0.90f, 0.85f, 0.70f, 0.80f, 0.75f, 0.86f,
        },
        .count = 16,
    };

    comp_error_rate_data_t error_rate = {
        .title = "Error rate",
        .value = "0.3%",
        .bars  = {
            0.10f, 0.15f, 0.08f, 0.20f, 0.12f, 0.09f,
            0.35f, 0.45f, 0.30f, 0.10f, 0.08f, 0.12f,
        },
        .count = 12,
    };

    comp_alerts_data_t alerts = {
        .title = "ALERTS",
        .rows  = {
            { "High memory usage on worker-03",     "3m ago",  XF_RGB(0xE24B4A), XF_RGBA(0xE24B4A, 0.06) },
            { "Elevated 5xx rate on /api/payments", "11m ago", XF_RGB(0xE24B4A), XF_RGBA(0xE24B4A, 0.06) },
            { "Cache hit ratio dropped below 80%",  "22m ago", XF_RGB(0xEF9F27), XF_RGBA(0xEF9F27, 0.06) },
        },
        .count = 3,
    };

    comp_outages_data_t outages = {
        .title = "ACTIVE OUTAGES",
        .rows  = {
            {
                "payments-svc", "2h 14m", "partial outage",
                XF_RGBA(0xE24B4A, 0.06),
                XF_RGB(0xF7C1C1), XF_RGB(0x791F1F),
                XF_RGB(0x501313), XF_RGB(0xE24B4A),
            },
            {
                "cdn-edge-eu", "47m", "degraded",
                XF_RGBA(0xEF9F27, 0.06),
                XF_RGB(0xFAC775), XF_RGB(0x633806),
                XF_RGB(0x412402), XF_RGB(0xEF9F27),
            },
        },
        .count = 2,
    };

    comp_sprint_data_t sprint = {
        .title          = "Sprint 24",
        .progress_label = "8 / 13 pts",
        .time_left      = "3d left",
        .percent        = 0.62f,
    };

    comp_team_status_data_t team = {
        .title   = "TEAM",
        .members = {
            { "MR", "Maya Rodriguez", XF_RGB(0x7F77DD), 1 },
            { "AK", "Alex Kim",       XF_RGB(0x1D9E75), 1 },
            { "JS", "Jordan Smith",   XF_RGB(0xE24B4A), 0 },
            { "PL", "Priya Lal",      XF_RGB(0xEF9F27), 1 },
            { "TW", "Tom Watkins",    XF_RGB(0x378ADD), 0 },
        },
        .count = 5,
    };

    comp_oncall_data_t oncall = {
        .initials     = "MR",
        .name         = "Maya Rodriguez",
        .role         = "Primary \xc2\xb7 platform",
        .phone        = "x4172",
        .avatar_color = XF_RGB(0x7F77DD),
    };

    comp_sla_gauge_data_t sla = {
        .title = "SLA ATTAINMENT",
        .rows  = {
            { "API Gateway",  "99.94%",  99.94f, XF_RGB(0x1D9E75) },
            { "Auth Service", "99.82%",  99.82f, XF_RGB(0x1D9E75) },
            { "DB Cluster",   "100.0%", 100.00f, XF_RGB(0x378ADD) },
        },
        .count = 3,
    };

    comp_schedule_data_t schedule = {
        .title = "TODAY",
        .rows  = {
            { "09:00-10:00", "Deploy freeze \xe2\x80\x94 release window", XF_RGB(0xE24B4A) },
            { "11:00-11:30", "Incident review: payments-svc",              XF_RGB(0xEF9F27) },
            { "14:00-15:00", "Architecture review: v3 API",                XF_RGB(0x378ADD) },
            { "17:00-17:15", "Daily standup",                              XF_RGB(0x1D9E75) },
        },
        .count = 4,
    };

    comp_pr_review_data_t pr = {
        .title = "NEEDS REVIEW",
        .rows  = {
            { "AK", "feat: add payment retry logic",     "2d", 2, XF_RGB(0x1D9E75) },
            { "JS", "fix: memory leak in session store", "5h", 0, XF_RGB(0xE24B4A) },
            { "PL", "chore: bump k8s chart to 1.28",     "1d", 1, XF_RGB(0xEF9F27) },
        },
        .count = 3,
    };

    comp_checklist_data_t checklist = {
        .title = "RELEASE CHECKLIST",
        .items = {
            { "Run integration test suite", 1 },
            { "Update CHANGELOG",           1 },
            { "Tag release in git",         0 },
            { "Notify stakeholders",        0 },
        },
        .count = 4,
    };

    xf_component_t c_header     = comp_header_create(&header);
    xf_component_t c_deploy     = comp_deploy_create(&deploy);
    xf_component_t c_build      = comp_build_status_create(&build);
    xf_component_t c_metrics    = comp_metrics_create(&metrics);
    xf_component_t c_sparkline  = comp_sparkline_create(&sparkline);
    xf_component_t c_error_rate = comp_error_rate_create(&error_rate);
    xf_component_t c_alerts     = comp_alerts_create(&alerts);
    xf_component_t c_outages    = comp_outages_create(&outages);
    xf_component_t c_sprint     = comp_sprint_create(&sprint);
    xf_component_t c_team       = comp_team_status_create(&team);
    xf_component_t c_oncall     = comp_oncall_create(&oncall);
    xf_component_t c_sla        = comp_sla_gauge_create(&sla);
    xf_component_t c_schedule   = comp_schedule_create(&schedule);
    xf_component_t c_pr         = comp_pr_review_create(&pr);
    xf_component_t c_checklist  = comp_checklist_create(&checklist);

    xf_component_t c_div_a = comp_divider_create();
    xf_component_t c_div_b = comp_divider_create();
    xf_component_t c_div_c = comp_divider_create();
    xf_component_t c_div_d = comp_divider_create();
    xf_component_t c_div_e = comp_divider_create();
    xf_component_t c_div_f = comp_divider_create();

    xf_component_t c_sp_a = comp_spacer_create();
    xf_component_t c_sp_b = comp_spacer_create();
    xf_component_t c_sp_c = comp_spacer_create();
    xf_component_t c_sp_d = comp_spacer_create();
    xf_component_t c_sp_e = comp_spacer_create();
    xf_component_t c_sp_f = comp_spacer_create();

    xf_dashboard_t *dash = dashboard_create(DISPLAY_W, DISPLAY_H, 4);
    if (!dash) {
        fprintf(stderr, "dashboard_create failed\n");
        return 1;
    }

    dashboard_add_full_row(dash, &c_header,     COMP_HEADER_HEIGHT);
    dashboard_add_full_row(dash, &c_sp_a,       COMP_SPACER_HEIGHT);
    dashboard_add_full_row(dash, &c_deploy,     COMP_DEPLOY_HEIGHT);
    dashboard_add_full_row(dash, &c_div_a,      COMP_DIVIDER_HEIGHT);
    dashboard_add_full_row(dash, &c_build,      COMP_BUILD_STATUS_HEIGHT);
    dashboard_add_full_row(dash, &c_div_b,      COMP_DIVIDER_HEIGHT);
    dashboard_add_full_row(dash, &c_metrics,    COMP_METRICS_HEIGHT);
    dashboard_add_full_row(dash, &c_sp_b,       COMP_SPACER_HEIGHT);
    dashboard_add_full_row(dash, &c_sparkline,  COMP_SPARKLINE_HEIGHT);
    dashboard_add_full_row(dash, &c_error_rate, COMP_ERROR_RATE_HEIGHT);

    dashboard_add_full_row(dash, &c_alerts,     COMP_ALERTS_HEIGHT);
    dashboard_add_full_row(dash, &c_sp_c,       COMP_SPACER_HEIGHT);
    dashboard_add_full_row(dash, &c_outages,    COMP_OUTAGES_HEIGHT);
    dashboard_add_full_row(dash, &c_sp_d,       COMP_SPACER_HEIGHT);
    dashboard_add_full_row(dash, &c_sprint,     COMP_SPRINT_HEIGHT);

    dashboard_add_full_row(dash, &c_sp_e,       COMP_SPACER_HEIGHT);
    dashboard_add_full_row(dash, &c_team,       COMP_TEAM_STATUS_HEIGHT);
    dashboard_add_full_row(dash, &c_div_c,      COMP_DIVIDER_HEIGHT);
    dashboard_add_full_row(dash, &c_oncall,     COMP_ONCALL_HEIGHT);
    dashboard_add_full_row(dash, &c_div_d,      COMP_DIVIDER_HEIGHT);
    dashboard_add_full_row(dash, &c_sla,        COMP_SLA_GAUGE_HEIGHT);
    dashboard_add_full_row(dash, &c_div_e,      COMP_DIVIDER_HEIGHT);
    dashboard_add_full_row(dash, &c_schedule,   COMP_SCHEDULE_HEIGHT);

    dashboard_add_full_row(dash, &c_sp_f,       COMP_SPACER_HEIGHT);
    dashboard_add_full_row(dash, &c_pr,         COMP_PR_REVIEW_HEIGHT);
    dashboard_add_full_row(dash, &c_div_f,      COMP_DIVIDER_HEIGHT);
    dashboard_add_full_row(dash, &c_checklist,  COMP_CHECKLIST_HEIGHT);

    int page_count = dashboard_page_count(dash);
    printf("Dashboard: %d pages\n", page_count);

    xf_device_t *dev = panel_open_auto();
    if (!dev)
        fprintf(stderr, "No device found; running in headless mode.\n");
    else {
        panel_set_orientation(dev, XF_ORIENT_LANDSCAPE);
        panel_set_brightness(dev, 80);
    }

    /* Send the first page without a transition and prime old_frame_buf. */
    {
        const uint8_t *f = dashboard_render_page(dash, 0);
        send_rect(dev, f, 0, 0, DISPLAY_W, DISPLAY_H);
        memcpy(old_frame_buf, f, (size_t)(DISPLAY_W * DISPLAY_H * 3));
        int _x, _y, _w, _h;
        dashboard_dirty_rect(dash, 0, &_x, &_y, &_w, &_h);
    }

    int              tick           = 0;
    int              page           = 0;
    int              page_changed   = 0;
    xf_transition_t  current_effect = XF_TRANS_WIPE_TOP;

    int alert_s[3]  = { 3 * 60,  11 * 60, 22 * 60 };
    int outage_s[2] = { 8040,    2820              };
    int deploy_s    = 8 * 60;
    int pr_h[3]     = { 48, 5, 24                  };

    while (1) {
        float rps_f = 412.0f + 40.0f * sinf((float)tick * 0.3f)
                    + 5.0f   * (float)(rand() % 11 - 5) / 5.0f;
        int   rps   = (int)rps_f;

        float p99_f = 142.0f + 30.0f * cosf((float)tick * 0.2f)
                    + (float)(rand() % 11 - 5);

        float err_f = 0.3f + 0.15f * sinf((float)tick * 0.4f + 1.0f);
        if (err_f < 0.0f) err_f = 0.0f;

        snprintf(metrics.cards[2].value, sizeof(metrics.cards[2].value), "%d",     rps);
        snprintf(metrics.cards[1].value, sizeof(metrics.cards[1].value), "%dms",   (int)p99_f);
        snprintf(metrics.cards[3].value, sizeof(metrics.cards[3].value), "%.1f%%", (double)err_f);
        c_metrics.dirty   = 1;

        memmove(sparkline.points, sparkline.points + 1,
                (size_t)(sparkline.count - 1) * sizeof(float));
        float sp = (rps_f - 300.0f) / 250.0f;
        sparkline.points[sparkline.count - 1] = sp < 0.0f ? 0.0f : (sp > 1.0f ? 1.0f : sp);
        snprintf(sparkline.value, sizeof(sparkline.value), "%d", rps);
        c_sparkline.dirty  = 1;

        memmove(error_rate.bars, error_rate.bars + 1,
                (size_t)(error_rate.count - 1) * sizeof(float));
        error_rate.bars[error_rate.count - 1] = err_f;
        snprintf(error_rate.value, sizeof(error_rate.value), "%.1f%%", (double)err_f);
        c_error_rate.dirty = 1;

        for (int i = 0; i < 3; i++) {
            alert_s[i] += 2;
            fmt_ago(alerts.rows[i].time, sizeof(alerts.rows[i].time), alert_s[i]);
        }
        c_alerts.dirty = 1;

        for (int i = 0; i < 2; i++) {
            outage_s[i] += 2;
            fmt_dur(outages.rows[i].duration, sizeof(outages.rows[i].duration), outage_s[i]);
        }
        c_outages.dirty = 1;

        deploy_s += 2;
        fmt_ago(deploy.time_ago, sizeof(deploy.time_ago), deploy_s);
        c_deploy.dirty = 1;

        float pts = 8.0f + (float)tick / 120.0f;
        if (pts > 13.0f) pts = 13.0f;
        sprint.percent = pts / 13.0f;
        snprintf(sprint.progress_label, sizeof(sprint.progress_label), "%d / 13 pts", (int)pts);
        c_sprint.dirty = 1;

        if (tick == 15 || tick == 30 || tick == 60 || tick == 75) {
            if (tick == 15) team.members[2].online = 1;
            if (tick == 30) team.members[4].online = 1;
            if (tick == 60) team.members[2].online = 0;
            if (tick == 75) team.members[4].online = 0;
            c_team.dirty = 1;
        }

        sla.rows[0].percent = 99.94f + 0.04f * sinf((float)tick * 0.1f);
        snprintf(sla.rows[0].value, sizeof(sla.rows[0].value), "%.2f%%", (double)sla.rows[0].percent);
        sla.rows[1].percent = 99.82f + 0.06f * cosf((float)tick * 0.15f);
        snprintf(sla.rows[1].value, sizeof(sla.rows[1].value), "%.2f%%", (double)sla.rows[1].percent);
        c_sla.dirty = 1;

        if (tick > 0 && tick % PAGE_TICKS == 0) {
            pr_h[1]++;
            fmt_pr_age(pr.rows[1].age, sizeof(pr.rows[1].age), pr_h[1]);
            c_pr.dirty = 1;
        }

        if (tick == 45) { checklist.items[2].done = 1; c_checklist.dirty = 1; }
        if (tick == 90) { checklist.items[3].done = 1; c_checklist.dirty = 1; }

        int ac = tick < 20 ? 2 : tick < 40 ? 1 : 0;
        if (ac == 2) {
            snprintf(header.status_text, sizeof(header.status_text), "2 alerts");
            header.status_dot = COL_RED;
        } else if (ac == 1) {
            snprintf(header.status_text, sizeof(header.status_text), "1 alert");
            header.status_dot = COL_ORG;
        } else {
            snprintf(header.status_text, sizeof(header.status_text), "all clear");
            header.status_dot = COL_GRN;
        }
        c_header.dirty = 1;

        const uint8_t *frame = dashboard_render_page(dash, page);

        send_ctx_t sc = { dev, frame };

        if (page_changed) {
            transition_play(dev, old_frame_buf, frame,
                            DISPLAY_W, DISPLAY_H, current_effect);
            current_effect = (xf_transition_t)((current_effect + 1) % XF_TRANS_COUNT);
            dashboard_visit_dirty_rects(dash, page, on_dirty_rect, NULL);
            page_changed = 0;
        } else {
            dashboard_visit_dirty_rects(dash, page, on_dirty_rect, &sc);
        }

        memcpy(old_frame_buf, frame, (size_t)(DISPLAY_W * DISPLAY_H * 3));

        printf("page %d/%d  tick %-4d  rps %-4d  p99 %-4.0fms  err %.1f%%\n",
               page + 1, page_count, tick, rps, (double)p99_f, (double)err_f);

        usleep(UPDATE_US);
        tick++;
        if (tick % PAGE_TICKS == 0) {
            page = (page + 1) % page_count;
            page_changed = 1;
        }
    }
}

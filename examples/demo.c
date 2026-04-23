#include <stdio.h>
#include <unistd.h>

#include "panel.h"
#include "dashboard.h"
#include "components/draw.h"
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

#define DISPLAY_W       480
#define DISPLAY_H       320
#define PAGE_INTERVAL_S 10

int main(void)
{
    xf_dashboard_t *dash;
    xf_device_t    *dev;
    int             page_count, page;

    comp_header_data_t header_data = {
        .date        = "Thu \xc2\xb7 Apr 23",
        .status_text = "2 alerts",
        .status_dot  = XF_RGB(0xE24B4A),
    };

    comp_deploy_data_t deploy_data = {
        .branch   = "main@a3f9c1e",
        .time_ago = "8m",
        .label    = "prod green",
    };

    comp_build_status_data_t build_data = {
        .branch       = "main",
        .build_id     = "#1847",
        .duration     = "2m 34s",
        .status       = "passing",
        .status_color = XF_RGB(0x1D9E75),
        .status_fg    = XF_RGB(0xFFFFFF),
    };

    comp_metrics_data_t metrics_data = {
        .cards = {
            { "UPTIME", "99.94%" },
            { "P99",    "142ms"  },
            { "RPS",    "412"    },
            { "ERRORS", "0.3%"   },
        },
        .count = 4,
    };

    comp_sparkline_data_t sparkline_data = {
        .title  = "req/s",
        .value  = "412",
        .points = {
            0.30f, 0.40f, 0.35f, 0.50f, 0.60f, 0.55f, 0.70f, 0.65f,
            0.80f, 0.75f, 0.90f, 0.85f, 0.70f, 0.80f, 0.75f, 0.86f,
        },
        .count = 16,
    };

    comp_error_rate_data_t error_data = {
        .title = "Error rate",
        .value = "0.3%",
        .bars  = {
            0.10f, 0.15f, 0.08f, 0.20f, 0.12f, 0.09f,
            0.35f, 0.45f, 0.30f, 0.10f, 0.08f, 0.12f,
        },
        .count = 12,
    };

    comp_alerts_data_t alerts_data = {
        .title = "ALERTS",
        .rows  = {
            {
                "High memory usage on worker-03",
                "3m ago",
                XF_RGB(0xE24B4A),
                XF_RGBA(0xE24B4A, 0.06),
            },
            {
                "Elevated 5xx rate on /api/payments",
                "11m ago",
                XF_RGB(0xE24B4A),
                XF_RGBA(0xE24B4A, 0.06),
            },
            {
                "Cache hit ratio dropped below 80%",
                "22m ago",
                XF_RGB(0xEF9F27),
                XF_RGBA(0xEF9F27, 0.06),
            },
        },
        .count = 3,
    };

    comp_outages_data_t outages_data = {
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

    comp_sprint_data_t sprint_data = {
        .title          = "Sprint 24",
        .progress_label = "8 / 13 pts",
        .time_left      = "3d left",
        .percent        = 0.62f,
    };

    comp_team_status_data_t team_data = {
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

    comp_oncall_data_t oncall_data = {
        .initials     = "MR",
        .name         = "Maya Rodriguez",
        .role         = "Primary \xc2\xb7 platform",
        .phone        = "x4172",
        .avatar_color = XF_RGB(0x7F77DD),
    };

    comp_sla_gauge_data_t sla_data = {
        .title = "SLA ATTAINMENT",
        .rows  = {
            { "API Gateway",  "99.94%",  99.94f, XF_RGB(0x1D9E75) },
            { "Auth Service", "99.82%",  99.82f, XF_RGB(0x1D9E75) },
            { "DB Cluster",   "100.0%", 100.00f, XF_RGB(0x378ADD) },
        },
        .count = 3,
    };

    comp_schedule_data_t schedule_data = {
        .title = "TODAY",
        .rows  = {
            { "09:00-10:00", "Deploy freeze \xe2\x80\x94 release window", XF_RGB(0xE24B4A) },
            { "11:00-11:30", "Incident review: payments-svc",              XF_RGB(0xEF9F27) },
            { "14:00-15:00", "Architecture review: v3 API",                XF_RGB(0x378ADD) },
            { "17:00-17:15", "Daily standup",                              XF_RGB(0x1D9E75) },
        },
        .count = 4,
    };

    comp_pr_review_data_t pr_data = {
        .title = "NEEDS REVIEW",
        .rows  = {
            { "AK", "feat: add payment retry logic",     "2d", 2, XF_RGB(0x1D9E75) },
            { "JS", "fix: memory leak in session store", "5h", 0, XF_RGB(0xE24B4A) },
            { "PL", "chore: bump k8s chart to 1.28",     "1d", 1, XF_RGB(0xEF9F27) },
        },
        .count = 3,
    };

    comp_checklist_data_t checklist_data = {
        .title = "RELEASE CHECKLIST",
        .items = {
            { "Run integration test suite", 1 },
            { "Update CHANGELOG",           1 },
            { "Tag release in git",         0 },
            { "Notify stakeholders",        0 },
        },
        .count = 4,
    };

    xf_component_t c_header     = comp_header_create(&header_data);
    xf_component_t c_deploy     = comp_deploy_create(&deploy_data);
    xf_component_t c_build      = comp_build_status_create(&build_data);
    xf_component_t c_metrics    = comp_metrics_create(&metrics_data);
    xf_component_t c_sparkline  = comp_sparkline_create(&sparkline_data);
    xf_component_t c_error_rate = comp_error_rate_create(&error_data);
    xf_component_t c_alerts     = comp_alerts_create(&alerts_data);
    xf_component_t c_outages    = comp_outages_create(&outages_data);
    xf_component_t c_sprint     = comp_sprint_create(&sprint_data);
    xf_component_t c_team       = comp_team_status_create(&team_data);
    xf_component_t c_oncall     = comp_oncall_create(&oncall_data);
    xf_component_t c_sla        = comp_sla_gauge_create(&sla_data);
    xf_component_t c_schedule   = comp_schedule_create(&schedule_data);
    xf_component_t c_pr         = comp_pr_review_create(&pr_data);
    xf_component_t c_checklist  = comp_checklist_create(&checklist_data);

    xf_component_t c_div_a  = comp_divider_create();
    xf_component_t c_div_b  = comp_divider_create();
    xf_component_t c_div_c  = comp_divider_create();
    xf_component_t c_div_d  = comp_divider_create();
    xf_component_t c_div_e  = comp_divider_create();
    xf_component_t c_div_f  = comp_divider_create();

    xf_component_t c_sp_a   = comp_spacer_create();
    xf_component_t c_sp_b   = comp_spacer_create();
    xf_component_t c_sp_c   = comp_spacer_create();
    xf_component_t c_sp_d   = comp_spacer_create();
    xf_component_t c_sp_e   = comp_spacer_create();
    xf_component_t c_sp_f   = comp_spacer_create();

    dash = dashboard_create(DISPLAY_W, DISPLAY_H, 4);
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

    page_count = dashboard_page_count(dash);
    printf("Dashboard: %d pages\n", page_count);

    dev = panel_open_auto();
    if (!dev)
        fprintf(stderr, "No device found; running in headless mode.\n");
    else {
        panel_set_orientation(dev, XF_ORIENT_LANDSCAPE);
        panel_set_brightness(dev, 80);
    }

    page = 0;
    while (1) {
        const uint8_t *frame = dashboard_render_page(dash, page);

        if (dev && panel_display_bitmap(dev, 0, 0, DISPLAY_W, DISPLAY_H, frame) < 0)
            fprintf(stderr, "panel_display_bitmap failed\n");

        printf("Page %d / %d\n", page + 1, page_count);
        sleep(PAGE_INTERVAL_S);
        page = (page + 1) % page_count;
    }
}

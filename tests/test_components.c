/**
 * @file test_components.c
 * @brief Component API tests — caller perspective only, no internal details.
 *
 * Every test follows the same pattern the real application uses:
 *   1. Allocate and fill a data struct.
 *   2. Create the component via its factory.
 *   3. Add it to a dashboard and call dashboard_render.
 *   4. Assert the returned buffer is non-NULL and contains at least one
 *      non-zero byte (confirming the component drew something).
 *   5. Write the buffer as a PPM to bin/ for visual inspection without hardware.
 *
 * PPM format is trivial: "P6\n<w> <h>\n255\n" followed by raw RGB888 bytes.
 * Open with any image viewer to confirm the layout looks correct.
 */

#include "vendor/unity.h"
#include "../src/dashboard.h"
#include "../src/components/draw.h"
#include "../src/components/comp_header.h"
#include "../src/components/comp_spacer.h"
#include "../src/components/comp_divider.h"
#include "../src/components/comp_metrics.h"
#include "../src/components/comp_oncall.h"
#include "../src/components/comp_deploy.h"
#include "../src/components/comp_outages.h"
#include "../src/components/comp_schedule.h"
#include "../src/components/comp_pr_review.h"
#include "../src/components/comp_sla_gauge.h"
#include "../src/components/comp_sparkline.h"
#include "../src/components/comp_sprint.h"
#include "../src/components/comp_alerts.h"
#include "../src/components/comp_error_rate.h"
#include "../src/components/comp_team_status.h"
#include "../src/components/comp_checklist.h"
#include "../src/components/comp_build_status.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

/* Matches the XuanFang display width. Tests use exactly the component height
   so each PPM file contains exactly one component — easier to inspect. */
#define DISPLAY_W 320

void setUp(void)   { xf_set_theme(&xf_theme_default); }
void tearDown(void) {}

static void write_ppm(const char *path, const uint8_t *buf, int w, int h)
{
    FILE *f = fopen(path, "wb");
    if (!f) return;
    fprintf(f, "P6\n%d %d\n255\n", w, h);
    fwrite(buf, 1, (size_t)(w * h * 3), f);
    fclose(f);
}

static int buffer_has_content(const uint8_t *buf, int w, int h)
{
    for (int i = 0; i < w * h * 3; i++)
        if (buf[i]) return 1;
    return 0;
}

/* Render a single full-width component into its own dashboard and return 1 if
   the buffer is non-NULL and non-zero. Writes a PPM side-effect. */
static int render_component(xf_component_t *comp, int height, const char *ppm)
{
    xf_dashboard_t *dash = dashboard_create(DISPLAY_W, height, 0);
    if (!dash) return 0;

    if (dashboard_add_full_row(dash, comp, height) != 0) {
        dashboard_destroy(dash);
        return 0;
    }

    const uint8_t *buf = dashboard_render(dash);
    int ok = buf && buffer_has_content(buf, DISPLAY_W, height);
    if (buf) write_ppm(ppm, buf, DISPLAY_W, height);

    dashboard_destroy(dash);
    return ok;
}

/* ── comp_header ─────────────────────────────────────────────────────────── */

static void test_header(void)
{
    const xf_theme_t *t = xf_get_theme();

    comp_header_data_t d = {0};
    snprintf(d.date, sizeof(d.date), "Mon · Apr 20");
    snprintf(d.status_text, sizeof(d.status_text), "2 active");
    d.status_dot = t->danger;

    xf_component_t comp = comp_header_create(&d);
    TEST_ASSERT_TRUE(render_component(&comp, COMP_HEADER_HEIGHT, "bin/test_header.ppm"));
}

/* ── comp_spacer ─────────────────────────────────────────────────────────── */

static void test_spacer(void)
{
    /* Spacer is intentionally blank; the buffer itself must still be allocated. */
    xf_dashboard_t *dash = dashboard_create(DISPLAY_W, COMP_SPACER_HEIGHT, 0);
    TEST_ASSERT_NOT_NULL(dash);

    xf_component_t comp = comp_spacer_create();
    TEST_ASSERT_EQUAL_INT(0, dashboard_add_full_row(dash, &comp, COMP_SPACER_HEIGHT));

    const uint8_t *buf = dashboard_render(dash);
    TEST_ASSERT_NOT_NULL(buf);

    dashboard_destroy(dash);
}

/* ── comp_divider ────────────────────────────────────────────────────────── */

static void test_divider(void)
{
    xf_component_t comp = comp_divider_create();
    /* A 1-pixel tall render is still a valid non-NULL buffer. */
    xf_dashboard_t *dash = dashboard_create(DISPLAY_W, COMP_DIVIDER_HEIGHT, 0);
    TEST_ASSERT_NOT_NULL(dash);
    TEST_ASSERT_EQUAL_INT(0, dashboard_add_full_row(dash, &comp, COMP_DIVIDER_HEIGHT));
    TEST_ASSERT_NOT_NULL(dashboard_render(dash));
    dashboard_destroy(dash);
}

/* ── comp_metrics ────────────────────────────────────────────────────────── */

static void test_metrics(void)
{
    comp_metrics_data_t d = {0};
    d.count = 3;
    snprintf(d.cards[0].label, sizeof(d.cards[0].label), "UPTIME");
    snprintf(d.cards[0].value, sizeof(d.cards[0].value), "99.94%%");
    snprintf(d.cards[1].label, sizeof(d.cards[1].label), "P50");
    snprintf(d.cards[1].value, sizeof(d.cards[1].value), "42ms");
    snprintf(d.cards[2].label, sizeof(d.cards[2].label), "ERR");
    snprintf(d.cards[2].value, sizeof(d.cards[2].value), "0.3%%");

    xf_component_t comp = comp_metrics_create(&d);
    TEST_ASSERT_TRUE(render_component(&comp, COMP_METRICS_HEIGHT, "bin/test_metrics.ppm"));
}

/* ── comp_oncall ─────────────────────────────────────────────────────────── */

static void test_oncall(void)
{
    const xf_theme_t *t = xf_get_theme();

    comp_oncall_data_t d = {0};
    snprintf(d.initials, sizeof(d.initials), "MR");
    snprintf(d.name,     sizeof(d.name),     "Maya Rodriguez");
    snprintf(d.role,     sizeof(d.role),     "Primary · platform");
    snprintf(d.phone,    sizeof(d.phone),    "x4172");
    d.avatar_color = t->accent;

    xf_component_t comp = comp_oncall_create(&d);
    TEST_ASSERT_TRUE(render_component(&comp, COMP_ONCALL_HEIGHT, "bin/test_oncall.ppm"));
}

/* ── comp_deploy ─────────────────────────────────────────────────────────── */

static void test_deploy(void)
{
    comp_deploy_data_t d = {0};
    snprintf(d.branch,   sizeof(d.branch),   "main@a3f2c1");
    snprintf(d.time_ago, sizeof(d.time_ago), "8m");
    snprintf(d.label,    sizeof(d.label),    "prod green");

    xf_component_t comp = comp_deploy_create(&d);
    TEST_ASSERT_TRUE(render_component(&comp, COMP_DEPLOY_HEIGHT, "bin/test_deploy.ppm"));
}

/* ── comp_outages ────────────────────────────────────────────────────────── */

static void test_outages(void)
{
    const xf_theme_t *t = xf_get_theme();

    comp_outages_data_t d = {0};
    snprintf(d.title, sizeof(d.title), "Active outages");
    d.count = 2;

    snprintf(d.rows[0].service,  sizeof(d.rows[0].service),  "auth-service");
    snprintf(d.rows[0].duration, sizeof(d.rows[0].duration), "2h 14m");
    snprintf(d.rows[0].status,   sizeof(d.rows[0].status),   "partial outage");
    d.rows[0].row_bg    = t->danger_bg;
    d.rows[0].pill_bg   = t->danger_pill_bg;
    d.rows[0].pill_fg   = t->danger_pill_fg;
    d.rows[0].title_fg  = t->danger_title_fg;
    d.rows[0].dot       = t->danger;

    snprintf(d.rows[1].service,  sizeof(d.rows[1].service),  "cdn-edge");
    snprintf(d.rows[1].duration, sizeof(d.rows[1].duration), "34m");
    snprintf(d.rows[1].status,   sizeof(d.rows[1].status),   "degraded");
    d.rows[1].row_bg    = t->warning_bg;
    d.rows[1].pill_bg   = t->warning_pill_bg;
    d.rows[1].pill_fg   = t->warning_pill_fg;
    d.rows[1].title_fg  = t->warning_title_fg;
    d.rows[1].dot       = t->warning;

    xf_component_t comp = comp_outages_create(&d);
    TEST_ASSERT_TRUE(render_component(&comp, COMP_OUTAGES_HEIGHT, "bin/test_outages.ppm"));
}

/* ── comp_schedule ───────────────────────────────────────────────────────── */

static void test_schedule(void)
{
    const xf_theme_t *t = xf_get_theme();

    comp_schedule_data_t d = {0};
    snprintf(d.title, sizeof(d.title), "Today");
    d.count = 3;

    snprintf(d.rows[0].time,  sizeof(d.rows[0].time),  "09:00-10:00");
    snprintf(d.rows[0].event, sizeof(d.rows[0].event), "Deploy freeze");
    d.rows[0].bar = t->orange;

    snprintf(d.rows[1].time,  sizeof(d.rows[1].time),  "14:00-15:00");
    snprintf(d.rows[1].event, sizeof(d.rows[1].event), "Incident review");
    d.rows[1].bar = t->info;

    snprintf(d.rows[2].time,  sizeof(d.rows[2].time),  "16:30-17:00");
    snprintf(d.rows[2].event, sizeof(d.rows[2].event), "Sprint planning");
    d.rows[2].bar = t->accent;

    xf_component_t comp = comp_schedule_create(&d);
    TEST_ASSERT_TRUE(render_component(&comp, COMP_SCHEDULE_HEIGHT, "bin/test_schedule.ppm"));
}

/* ── comp_pr_review ──────────────────────────────────────────────────────── */

static void test_pr_review(void)
{
    const xf_theme_t *t = xf_get_theme();

    comp_pr_review_data_t d = {0};
    snprintf(d.title, sizeof(d.title), "PRs waiting");
    d.count = 2;

    snprintf(d.rows[0].initials, sizeof(d.rows[0].initials), "AR");
    snprintf(d.rows[0].title,    sizeof(d.rows[0].title),    "Add rate-limit middleware");
    snprintf(d.rows[0].age,      sizeof(d.rows[0].age),      "2d");
    d.rows[0].reviews      = 1;
    d.rows[0].avatar_color = t->info;

    snprintf(d.rows[1].initials, sizeof(d.rows[1].initials), "KL");
    snprintf(d.rows[1].title,    sizeof(d.rows[1].title),    "Fix token expiry bug");
    snprintf(d.rows[1].age,      sizeof(d.rows[1].age),      "5h");
    d.rows[1].reviews      = 0;
    d.rows[1].avatar_color = t->accent;

    xf_component_t comp = comp_pr_review_create(&d);
    TEST_ASSERT_TRUE(render_component(&comp, COMP_PR_REVIEW_HEIGHT, "bin/test_pr_review.ppm"));
}

/* ── comp_sla_gauge ──────────────────────────────────────────────────────── */

static void test_sla_gauge(void)
{
    const xf_theme_t *t = xf_get_theme();

    comp_sla_gauge_data_t d = {0};
    snprintf(d.title, sizeof(d.title), "SLA attainment");
    d.count = 3;

    snprintf(d.rows[0].label, sizeof(d.rows[0].label), "API Gateway");
    snprintf(d.rows[0].value, sizeof(d.rows[0].value), "99.94%%");
    d.rows[0].percent = 99.94f;
    d.rows[0].bar     = t->success;

    snprintf(d.rows[1].label, sizeof(d.rows[1].label), "Auth service");
    snprintf(d.rows[1].value, sizeof(d.rows[1].value), "98.1%%");
    d.rows[1].percent = 98.1f;
    d.rows[1].bar     = t->warning;

    snprintf(d.rows[2].label, sizeof(d.rows[2].label), "CDN edge");
    snprintf(d.rows[2].value, sizeof(d.rows[2].value), "95.2%%");
    d.rows[2].percent = 95.2f;
    d.rows[2].bar     = t->danger;

    xf_component_t comp = comp_sla_gauge_create(&d);
    TEST_ASSERT_TRUE(render_component(&comp, COMP_SLA_GAUGE_HEIGHT, "bin/test_sla_gauge.ppm"));
}

/* ── comp_sparkline ──────────────────────────────────────────────────────── */

static void test_sparkline(void)
{
    comp_sparkline_data_t d = {0};
    snprintf(d.title, sizeof(d.title), "req/s");
    snprintf(d.value, sizeof(d.value), "412");

    /* Sine-like pattern to exercise both the fill area and the top-point dot */
    float wave[] = {0.4f, 0.5f, 0.7f, 0.6f, 0.8f, 0.9f, 0.7f,
                    0.5f, 0.3f, 0.4f, 0.6f, 0.8f, 1.0f, 0.9f, 0.7f};
    d.count = (int)(sizeof(wave) / sizeof(wave[0]));
    for (int i = 0; i < d.count; i++)
        d.points[i] = wave[i];

    xf_component_t comp = comp_sparkline_create(&d);
    TEST_ASSERT_TRUE(render_component(&comp, COMP_SPARKLINE_HEIGHT, "bin/test_sparkline.ppm"));
}

/* ── comp_sprint ─────────────────────────────────────────────────────────── */

static void test_sprint(void)
{
    comp_sprint_data_t d = {0};
    snprintf(d.title,          sizeof(d.title),          "Sprint 24");
    snprintf(d.progress_label, sizeof(d.progress_label), "8 / 13 pts");
    snprintf(d.time_left,      sizeof(d.time_left),       "3d left");
    d.percent = 0.615f;

    xf_component_t comp = comp_sprint_create(&d);
    TEST_ASSERT_TRUE(render_component(&comp, COMP_SPRINT_HEIGHT, "bin/test_sprint.ppm"));
}

/* ── comp_alerts ─────────────────────────────────────────────────────────── */

static void test_alerts(void)
{
    const xf_theme_t *t = xf_get_theme();

    comp_alerts_data_t d = {0};
    snprintf(d.title, sizeof(d.title), "Recent alerts");
    d.count = 3;

    snprintf(d.rows[0].message, sizeof(d.rows[0].message), "CPU spike on web-01");
    snprintf(d.rows[0].time,    sizeof(d.rows[0].time),    "3m ago");
    d.rows[0].dot    = t->danger;
    d.rows[0].row_bg = t->danger_bg;

    snprintf(d.rows[1].message, sizeof(d.rows[1].message), "High memory on db-02");
    snprintf(d.rows[1].time,    sizeof(d.rows[1].time),    "11m ago");
    d.rows[1].dot    = t->warning;
    d.rows[1].row_bg = t->warning_bg;

    snprintf(d.rows[2].message, sizeof(d.rows[2].message), "Deploy started");
    snprintf(d.rows[2].time,    sizeof(d.rows[2].time),    "18m ago");
    d.rows[2].dot    = t->info;
    d.rows[2].row_bg = t->info_bg;

    xf_component_t comp = comp_alerts_create(&d);
    TEST_ASSERT_TRUE(render_component(&comp, COMP_ALERTS_HEIGHT, "bin/test_alerts.ppm"));
}

/* ── comp_error_rate ─────────────────────────────────────────────────────── */

static void test_error_rate(void)
{
    comp_error_rate_data_t d = {0};
    snprintf(d.title, sizeof(d.title), "Error rate");
    snprintf(d.value, sizeof(d.value), "0.3%%");
    d.count = 14;

    float heights[] = {0.1f,0.2f,0.15f,0.3f,0.25f,0.4f,0.5f,
                       0.6f,0.45f,0.3f,0.8f,0.9f,0.7f,0.5f};
    for (int i = 0; i < d.count; i++)
        d.bars[i] = heights[i];

    xf_component_t comp = comp_error_rate_create(&d);
    TEST_ASSERT_TRUE(render_component(&comp, COMP_ERROR_RATE_HEIGHT, "bin/test_error_rate.ppm"));
}

/* ── comp_team_status ────────────────────────────────────────────────────── */

static void test_team_status(void)
{
    const xf_theme_t *t = xf_get_theme();

    comp_team_status_data_t d = {0};
    snprintf(d.title, sizeof(d.title), "On shift");
    d.count = 4;

    snprintf(d.members[0].initials, sizeof(d.members[0].initials), "MR");
    snprintf(d.members[0].name,     sizeof(d.members[0].name),     "Maya Rodriguez");
    d.members[0].avatar_color = t->info;
    d.members[0].online       = 1;

    snprintf(d.members[1].initials, sizeof(d.members[1].initials), "KL");
    snprintf(d.members[1].name,     sizeof(d.members[1].name),     "Kim Lee");
    d.members[1].avatar_color = t->accent;
    d.members[1].online       = 1;

    snprintf(d.members[2].initials, sizeof(d.members[2].initials), "JB");
    snprintf(d.members[2].name,     sizeof(d.members[2].name),     "James Brown");
    d.members[2].avatar_color = t->orange;
    d.members[2].online       = 0;

    snprintf(d.members[3].initials, sizeof(d.members[3].initials), "AP");
    snprintf(d.members[3].name,     sizeof(d.members[3].name),     "Ana Pinto");
    d.members[3].avatar_color = t->success;
    d.members[3].online       = 1;

    xf_component_t comp = comp_team_status_create(&d);
    TEST_ASSERT_TRUE(render_component(&comp, COMP_TEAM_STATUS_HEIGHT, "bin/test_team_status.ppm"));
}

/* ── comp_checklist ──────────────────────────────────────────────────────── */

static void test_checklist(void)
{
    comp_checklist_data_t d = {0};
    snprintf(d.title, sizeof(d.title), "Release checklist");
    d.count = 4;

    snprintf(d.items[0].item, sizeof(d.items[0].item), "Smoke tests passed");
    d.items[0].done = 1;

    snprintf(d.items[1].item, sizeof(d.items[1].item), "DB migration applied");
    d.items[1].done = 1;

    snprintf(d.items[2].item, sizeof(d.items[2].item), "Rollback plan verified");
    d.items[2].done = 0;

    snprintf(d.items[3].item, sizeof(d.items[3].item), "Stakeholders notified");
    d.items[3].done = 0;

    xf_component_t comp = comp_checklist_create(&d);
    TEST_ASSERT_TRUE(render_component(&comp, COMP_CHECKLIST_HEIGHT, "bin/test_checklist.ppm"));
}

/* ── comp_build_status ───────────────────────────────────────────────────── */

static void test_build_status(void)
{
    const xf_theme_t *t = xf_get_theme();

    comp_build_status_data_t d = {0};
    snprintf(d.branch,   sizeof(d.branch),   "main");
    snprintf(d.build_id, sizeof(d.build_id), "#1234");
    snprintf(d.duration, sizeof(d.duration), "2m 34s");
    snprintf(d.status,   sizeof(d.status),   "passing");
    d.status_color = t->success_bg;
    d.status_fg    = t->success_fg;

    xf_component_t comp = comp_build_status_create(&d);
    TEST_ASSERT_TRUE(render_component(&comp, COMP_BUILD_STATUS_HEIGHT, "bin/test_build_status.ppm"));
}

/* ── custom theme ────────────────────────────────────────────────────────── */

/* Confirms that no component hard-codes colour literals: switching to a
   visually distinct theme should still produce a non-zero buffer. */
static void test_custom_theme(void)
{
    xf_theme_t dark = xf_theme_default;
    dark.text_primary   = (xf_rgba_t)XF_RGB(0xEEEEEE);
    dark.text_secondary = (xf_rgba_t)XF_RGB(0xCCCCCC);
    dark.text_muted     = (xf_rgba_t)XF_RGB(0x999999);
    dark.surface_card   = (xf_rgba_t)XF_RGB(0x2A2A2A);
    xf_set_theme(&dark);

    comp_header_data_t hdr = {0};
    snprintf(hdr.date,        sizeof(hdr.date),        "Mon · Apr 20");
    snprintf(hdr.status_text, sizeof(hdr.status_text), "all good");
    hdr.status_dot = dark.success;

    xf_component_t comp = comp_header_create(&hdr);
    TEST_ASSERT_TRUE(render_component(&comp, COMP_HEADER_HEIGHT, "bin/test_header_dark.ppm"));

    xf_set_theme(&xf_theme_default);
}

int main(void)
{
    UNITY_BEGIN();

    RUN_TEST(test_header);
    RUN_TEST(test_spacer);
    RUN_TEST(test_divider);
    RUN_TEST(test_metrics);
    RUN_TEST(test_oncall);
    RUN_TEST(test_deploy);
    RUN_TEST(test_outages);
    RUN_TEST(test_schedule);
    RUN_TEST(test_pr_review);
    RUN_TEST(test_sla_gauge);
    RUN_TEST(test_sparkline);
    RUN_TEST(test_sprint);
    RUN_TEST(test_alerts);
    RUN_TEST(test_error_rate);
    RUN_TEST(test_team_status);
    RUN_TEST(test_checklist);
    RUN_TEST(test_build_status);
    RUN_TEST(test_custom_theme);

    return UNITY_END();
}

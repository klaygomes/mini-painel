#include "vendor/unity.h"
#include "../src/dashboard.h"
#include "../src/components/draw.h"

#include <string.h>
#include <stddef.h>

#define TO_R(c) ((uint8_t)((c).r * 255.0f))
#define TO_G(c) ((uint8_t)((c).g * 255.0f))
#define TO_B(c) ((uint8_t)((c).b * 255.0f))

/* Display dimensions used across all tests. */
#define W 100
#define H 60

/* ── test helpers ─────────────────────────────────────────────────────────── */

/* Pixel accessor: byte offset of channel ch at (x, y) in a W-wide framebuffer. */
#define PX(fb, x, y, ch) ((fb)[((y) * W + (x)) * 3 + (ch)])

/* Solid-colour render callbacks. */
static void render_red(xf_component_t *self, uint8_t *buf, int w, int h)
{
    int i;
    (void)self;
    for (i = 0; i < w * h; i++) { buf[i*3]=0xFF; buf[i*3+1]=0x00; buf[i*3+2]=0x00; }
}

static void render_blue(xf_component_t *self, uint8_t *buf, int w, int h)
{
    int i;
    (void)self;
    for (i = 0; i < w * h; i++) { buf[i*3]=0x00; buf[i*3+1]=0x00; buf[i*3+2]=0xFF; }
}

static void render_green(xf_component_t *self, uint8_t *buf, int w, int h)
{
    int i;
    (void)self;
    for (i = 0; i < w * h; i++) { buf[i*3]=0x00; buf[i*3+1]=0xFF; buf[i*3+2]=0x00; }
}

/* Helpers that record the dimensions received by render(). */
static int last_render_w = 0;
static int last_render_h = 0;

static void render_record_size(xf_component_t *self, uint8_t *buf, int w, int h)
{
    (void)self; (void)buf;
    last_render_w = w;
    last_render_h = h;
}

/* Helpers that test fetch-before-render ordering. */
static int fetch_ran  = 0;
static int render_saw_fetch = 0;

static int fetch_set_flag(xf_component_t *self)
{
    (void)self;
    fetch_ran = 1;
    return 0;
}

static void render_check_flag(xf_component_t *self, uint8_t *buf, int w, int h)
{
    (void)self; (void)buf; (void)w; (void)h;
    render_saw_fetch = fetch_ran;
}

/* Fetch that signals an error. */
static int fetch_error(xf_component_t *self)
{
    (void)self;
    return -1;
}

static int render_was_called = 0;

static void render_set_called(xf_component_t *self, uint8_t *buf, int w, int h)
{
    (void)self; (void)buf; (void)w; (void)h;
    render_was_called = 1;
}

/* ── setUp / tearDown ─────────────────────────────────────────────────────── */

void setUp(void)
{
    fetch_ran        = 0;
    render_saw_fetch = 0;
    render_was_called = 0;
    last_render_w    = 0;
    last_render_h    = 0;
}

void tearDown(void) {}

/* ── creation / destruction ───────────────────────────────────────────────── */

static void test_create_returns_non_null_for_valid_dimensions(void)
{
    xf_dashboard_t *dash = dashboard_create(W, H, 0);
    TEST_ASSERT_NOT_NULL(dash);
    dashboard_destroy(dash);
}

static void test_destroy_null_is_a_noop(void)
{
    /* Must not crash. */
    dashboard_destroy(NULL);
}

/* ── dashboard_add_row rejection ──────────────────────────────────────────── */

static void test_add_row_rejects_null_dashboard(void)
{
    xf_component_t comp = {NULL, render_red, NULL, 0};
    xf_component_t *comps[] = {&comp};
    int widths[] = {W};
    TEST_ASSERT_EQUAL_INT(-1, dashboard_add_row(NULL, comps, widths, 1, 10));
}

static void test_add_row_rejects_null_components(void)
{
    xf_dashboard_t *dash = dashboard_create(W, H, 0);
    int widths[] = {W};
    TEST_ASSERT_EQUAL_INT(-1, dashboard_add_row(dash, NULL, widths, 1, 10));
    dashboard_destroy(dash);
}

static void test_add_row_rejects_null_widths(void)
{
    xf_dashboard_t *dash = dashboard_create(W, H, 0);
    xf_component_t comp  = {NULL, render_red, NULL, 0};
    xf_component_t *comps[] = {&comp};
    TEST_ASSERT_EQUAL_INT(-1, dashboard_add_row(dash, comps, NULL, 1, 10));
    dashboard_destroy(dash);
}

static void test_add_row_rejects_zero_count(void)
{
    xf_dashboard_t *dash = dashboard_create(W, H, 0);
    xf_component_t comp  = {NULL, render_red, NULL, 0};
    xf_component_t *comps[] = {&comp};
    int widths[] = {W};
    TEST_ASSERT_EQUAL_INT(-1, dashboard_add_row(dash, comps, widths, 0, 10));
    dashboard_destroy(dash);
}

static void test_add_row_rejects_zero_height(void)
{
    xf_dashboard_t *dash = dashboard_create(W, H, 0);
    xf_component_t comp  = {NULL, render_red, NULL, 0};
    xf_component_t *comps[] = {&comp};
    int widths[] = {W};
    TEST_ASSERT_EQUAL_INT(-1, dashboard_add_row(dash, comps, widths, 1, 0));
    dashboard_destroy(dash);
}

static void test_add_row_rejects_widths_that_dont_sum_to_display_width(void)
{
    xf_dashboard_t *dash = dashboard_create(W, H, 0);
    xf_component_t comp  = {NULL, render_red, NULL, 0};
    xf_component_t *comps[] = {&comp};
    int widths[] = {W - 1}; /* intentionally wrong */
    TEST_ASSERT_EQUAL_INT(-1, dashboard_add_row(dash, comps, widths, 1, 10));
    dashboard_destroy(dash);
}

/* ── dashboard_add_full_row rejection ────────────────────────────────────── */

static void test_add_full_row_rejects_null_dashboard(void)
{
    xf_component_t comp = {NULL, render_red, NULL, 0};
    TEST_ASSERT_EQUAL_INT(-1, dashboard_add_full_row(NULL, &comp, 10));
}

static void test_add_full_row_rejects_null_component(void)
{
    xf_dashboard_t *dash = dashboard_create(W, H, 0);
    TEST_ASSERT_EQUAL_INT(-1, dashboard_add_full_row(dash, NULL, 10));
    dashboard_destroy(dash);
}

/* ── dashboard_render basics ─────────────────────────────────────────────── */

static void test_render_returns_null_for_null_dashboard(void)
{
    TEST_ASSERT_NULL(dashboard_render(NULL));
}

static void test_render_returns_non_null_for_valid_dashboard(void)
{
    xf_dashboard_t *dash = dashboard_create(W, H, 0);
    TEST_ASSERT_NOT_NULL(dashboard_render(dash));
    dashboard_destroy(dash);
}

/* ── fetch / render lifecycle ────────────────────────────────────────────── */

static void test_render_calls_fetch_before_render(void)
{
    xf_dashboard_t *dash = dashboard_create(W, H, 0);
    xf_component_t comp  = {fetch_set_flag, render_check_flag, NULL, 0};

    dashboard_add_full_row(dash, &comp, H);
    dashboard_render(dash);

    TEST_ASSERT_TRUE(render_saw_fetch);
    dashboard_destroy(dash);
}

static void test_render_skips_fetch_when_null(void)
{
    /* Component with NULL fetch must still have render() called. */
    xf_dashboard_t *dash = dashboard_create(W, H, 0);
    xf_component_t comp  = {NULL, render_set_called, NULL, 0};

    dashboard_add_full_row(dash, &comp, H);
    dashboard_render(dash);

    TEST_ASSERT_TRUE(render_was_called);
    dashboard_destroy(dash);
}

static void test_render_proceeds_when_fetch_returns_error(void)
{
    /* fetch returns -1 but render should still run. */
    xf_dashboard_t *dash = dashboard_create(W, H, 0);
    xf_component_t comp  = {fetch_error, render_set_called, NULL, 0};

    dashboard_add_full_row(dash, &comp, H);
    dashboard_render(dash);

    TEST_ASSERT_TRUE(render_was_called);
    dashboard_destroy(dash);
}

/* ── component receives correct dimensions ───────────────────────────────── */

static void test_render_passes_full_width_to_full_row_component(void)
{
    xf_dashboard_t *dash = dashboard_create(W, H, 0);
    xf_component_t comp  = {NULL, render_record_size, NULL, 0};

    dashboard_add_full_row(dash, &comp, H);
    dashboard_render(dash);

    TEST_ASSERT_EQUAL_INT(W, last_render_w);
    TEST_ASSERT_EQUAL_INT(H, last_render_h);
    dashboard_destroy(dash);
}

static void test_render_passes_half_width_to_split_row_component(void)
{
    xf_dashboard_t *dash = dashboard_create(W, H, 0);
    xf_component_t left  = {NULL, render_record_size, NULL, 0};
    xf_component_t right = {NULL, render_red, NULL, 0};
    xf_component_t *comps[] = {&left, &right};
    int widths[] = {W / 2, W / 2};

    dashboard_add_row(dash, comps, widths, 2, H);
    dashboard_render(dash);

    TEST_ASSERT_EQUAL_INT(W / 2, last_render_w);
    TEST_ASSERT_EQUAL_INT(H,     last_render_h);
    dashboard_destroy(dash);
}

/* ── pixel placement ─────────────────────────────────────────────────────── */

static void test_single_full_row_pixels_appear_at_top_left(void)
{
    xf_dashboard_t *dash = dashboard_create(W, H, 0);
    xf_component_t comp  = {NULL, render_red, NULL, 0};
    const uint8_t *fb;

    dashboard_add_full_row(dash, &comp, H);
    fb = dashboard_render(dash);

    TEST_ASSERT_EQUAL_UINT8(0xFF, PX(fb, 0, 0, 0)); /* R */
    TEST_ASSERT_EQUAL_UINT8(0x00, PX(fb, 0, 0, 1)); /* G */
    TEST_ASSERT_EQUAL_UINT8(0x00, PX(fb, 0, 0, 2)); /* B */
    dashboard_destroy(dash);
}

static void test_second_row_pixels_appear_below_first_row(void)
{
    /* Red row of height 10, blue row below it. */
    xf_dashboard_t *dash  = dashboard_create(W, H, 0);
    xf_component_t  red   = {NULL, render_red, NULL, 0};
    xf_component_t  blue  = {NULL, render_blue, NULL, 0};
    const uint8_t  *fb;

    dashboard_add_full_row(dash, &red,  10);
    dashboard_add_full_row(dash, &blue, H - 10);
    fb = dashboard_render(dash);

    /* Last row of the red region. */
    TEST_ASSERT_EQUAL_UINT8(0xFF, PX(fb, 0, 9, 0));
    TEST_ASSERT_EQUAL_UINT8(0x00, PX(fb, 0, 9, 2));

    /* First row of the blue region. */
    TEST_ASSERT_EQUAL_UINT8(0x00, PX(fb, 0, 10, 0));
    TEST_ASSERT_EQUAL_UINT8(0xFF, PX(fb, 0, 10, 2));
    dashboard_destroy(dash);
}

static void test_right_column_pixels_appear_at_correct_x_offset(void)
{
    /* Left half red, right half blue. */
    xf_dashboard_t *dash  = dashboard_create(W, H, 0);
    xf_component_t  left  = {NULL, render_red, NULL, 0};
    xf_component_t  right = {NULL, render_blue, NULL, 0};
    xf_component_t *comps[] = {&left, &right};
    int widths[] = {W / 2, W / 2};
    const uint8_t *fb;

    dashboard_add_row(dash, comps, widths, 2, H);
    fb = dashboard_render(dash);

    /* Last pixel of left column at y=0. */
    TEST_ASSERT_EQUAL_UINT8(0xFF, PX(fb, W/2 - 1, 0, 0)); /* red */
    TEST_ASSERT_EQUAL_UINT8(0x00, PX(fb, W/2 - 1, 0, 2));

    /* First pixel of right column at y=0. */
    TEST_ASSERT_EQUAL_UINT8(0x00, PX(fb, W/2, 0, 0));     /* blue */
    TEST_ASSERT_EQUAL_UINT8(0xFF, PX(fb, W/2, 0, 2));
    dashboard_destroy(dash);
}

static void test_framebuffer_is_cleared_between_renders(void)
{
    /* Render with a red row, remove it, render again — area is theme background. */
    xf_dashboard_t *dash = dashboard_create(W, H, 0);
    xf_component_t  comp = {NULL, render_red, NULL, 0};
    const uint8_t  *fb;

    dashboard_add_full_row(dash, &comp, H);
    dashboard_render(dash);

    dashboard_remove_row(dash, 0);
    fb = dashboard_render(dash);

    TEST_ASSERT_EQUAL_UINT8(TO_R(xf_get_theme()->background), PX(fb, 0, 0, 0));
    TEST_ASSERT_EQUAL_UINT8(TO_G(xf_get_theme()->background), PX(fb, 0, 0, 1));
    TEST_ASSERT_EQUAL_UINT8(TO_B(xf_get_theme()->background), PX(fb, 0, 0, 2));
    dashboard_destroy(dash);
}

/* ── row mutation ────────────────────────────────────────────────────────── */

static void test_move_row_up_swaps_visual_position(void)
{
    /* Add red (top) then blue (bottom), move blue up → blue on top. */
    xf_dashboard_t *dash = dashboard_create(W, H, 0);
    xf_component_t  red  = {NULL, render_red, NULL, 0};
    xf_component_t  blue = {NULL, render_blue, NULL, 0};
    const uint8_t  *fb;

    dashboard_add_full_row(dash, &red,  H / 2);
    dashboard_add_full_row(dash, &blue, H / 2);

    TEST_ASSERT_EQUAL_INT(0, dashboard_move_row_up(dash, 1));
    fb = dashboard_render(dash);

    /* Top row is now blue. */
    TEST_ASSERT_EQUAL_UINT8(0xFF, PX(fb, 0, 0, 2));
    TEST_ASSERT_EQUAL_UINT8(0x00, PX(fb, 0, 0, 0));
    dashboard_destroy(dash);
}

static void test_move_row_down_swaps_visual_position(void)
{
    /* Add red (top) then blue (bottom), move red down → blue on top. */
    xf_dashboard_t *dash = dashboard_create(W, H, 0);
    xf_component_t  red  = {NULL, render_red, NULL, 0};
    xf_component_t  blue = {NULL, render_blue, NULL, 0};
    const uint8_t  *fb;

    dashboard_add_full_row(dash, &red,  H / 2);
    dashboard_add_full_row(dash, &blue, H / 2);

    TEST_ASSERT_EQUAL_INT(0, dashboard_move_row_down(dash, 0));
    fb = dashboard_render(dash);

    /* Top row is now blue. */
    TEST_ASSERT_EQUAL_UINT8(0xFF, PX(fb, 0, 0, 2));
    TEST_ASSERT_EQUAL_UINT8(0x00, PX(fb, 0, 0, 0));
    dashboard_destroy(dash);
}

static void test_move_row_up_on_first_row_returns_error(void)
{
    xf_dashboard_t *dash = dashboard_create(W, H, 0);
    xf_component_t  comp = {NULL, render_red, NULL, 0};
    dashboard_add_full_row(dash, &comp, H);
    TEST_ASSERT_EQUAL_INT(-1, dashboard_move_row_up(dash, 0));
    dashboard_destroy(dash);
}

static void test_move_row_down_on_last_row_returns_error(void)
{
    xf_dashboard_t *dash = dashboard_create(W, H, 0);
    xf_component_t  comp = {NULL, render_red, NULL, 0};
    dashboard_add_full_row(dash, &comp, H);
    TEST_ASSERT_EQUAL_INT(-1, dashboard_move_row_down(dash, 0));
    dashboard_destroy(dash);
}

static void test_remove_row_eliminates_pixels_from_next_render(void)
{
    xf_dashboard_t *dash  = dashboard_create(W, H, 0);
    xf_component_t  red   = {NULL, render_red, NULL, 0};
    xf_component_t  green = {NULL, render_green, NULL, 0};
    const uint8_t  *fb;

    /* Two rows: red (top 10px), green (rest). */
    dashboard_add_full_row(dash, &red,   10);
    dashboard_add_full_row(dash, &green, H - 10);

    /* Remove the red row; green moves to the top. */
    TEST_ASSERT_EQUAL_INT(0, dashboard_remove_row(dash, 0));
    fb = dashboard_render(dash);

    /* Previously-red area (y=0) is now green. */
    TEST_ASSERT_EQUAL_UINT8(0x00, PX(fb, 0, 0, 0));
    TEST_ASSERT_EQUAL_UINT8(0xFF, PX(fb, 0, 0, 1));
    TEST_ASSERT_EQUAL_UINT8(0x00, PX(fb, 0, 0, 2));
    dashboard_destroy(dash);
}

static void test_remove_row_out_of_bounds_returns_error(void)
{
    xf_dashboard_t *dash = dashboard_create(W, H, 0);
    TEST_ASSERT_EQUAL_INT(-1, dashboard_remove_row(dash, 0));
    TEST_ASSERT_EQUAL_INT(-1, dashboard_remove_row(dash, -1));
    dashboard_destroy(dash);
}

/* ── pagination: dashboard_page_count ────────────────────────────────────── */

static void test_page_count_returns_zero_for_null(void)
{
    TEST_ASSERT_EQUAL_INT(0, dashboard_page_count(NULL));
}

static void test_page_count_is_one_for_empty_dashboard(void)
{
    xf_dashboard_t *dash = dashboard_create(W, H, 0);
    TEST_ASSERT_EQUAL_INT(1, dashboard_page_count(dash));
    dashboard_destroy(dash);
}

static void test_page_count_is_one_when_rows_fit_within(void)
{
    xf_dashboard_t *dash = dashboard_create(W, H, 0);
    xf_component_t  comp = {NULL, render_red, NULL, 0};
    dashboard_add_full_row(dash, &comp, H / 2);
    TEST_ASSERT_EQUAL_INT(1, dashboard_page_count(dash));
    dashboard_destroy(dash);
}

static void test_page_count_is_one_when_rows_fit_exactly(void)
{
    xf_dashboard_t *dash = dashboard_create(W, H, 0);
    xf_component_t  a    = {NULL, render_red, NULL, 0};
    xf_component_t  b    = {NULL, render_blue, NULL, 0};
    dashboard_add_full_row(dash, &a, H / 2);
    dashboard_add_full_row(dash, &b, H / 2); /* total = H exactly */
    TEST_ASSERT_EQUAL_INT(1, dashboard_page_count(dash));
    dashboard_destroy(dash);
}

static void test_page_count_is_two_when_rows_overflow(void)
{
    /* Row A (40px) fits page 0. Row B (40px) would exceed H=60, goes to page 1. */
    xf_dashboard_t *dash = dashboard_create(W, H, 0);
    xf_component_t  a    = {NULL, render_red, NULL, 0};
    xf_component_t  b    = {NULL, render_blue, NULL, 0};
    dashboard_add_full_row(dash, &a, 40);
    dashboard_add_full_row(dash, &b, 40);
    TEST_ASSERT_EQUAL_INT(2, dashboard_page_count(dash));
    dashboard_destroy(dash);
}

static void test_page_count_is_three_for_three_pages(void)
{
    /* Three rows of height 40 each: page 0, 1, 2. */
    xf_dashboard_t *dash = dashboard_create(W, H, 0);
    xf_component_t  a    = {NULL, render_red, NULL, 0};
    xf_component_t  b    = {NULL, render_blue, NULL, 0};
    xf_component_t  c    = {NULL, render_green, NULL, 0};
    dashboard_add_full_row(dash, &a, 40);
    dashboard_add_full_row(dash, &b, 40);
    dashboard_add_full_row(dash, &c, 40);
    TEST_ASSERT_EQUAL_INT(3, dashboard_page_count(dash));
    dashboard_destroy(dash);
}

/* ── pagination: dashboard_render_page ───────────────────────────────────── */

static void test_render_page_returns_null_for_null_dashboard(void)
{
    TEST_ASSERT_NULL(dashboard_render_page(NULL, 0));
}

static void test_render_page_0_shows_rows_fitting_display(void)
{
    xf_dashboard_t *dash = dashboard_create(W, H, 0);
    xf_component_t  comp = {NULL, render_red, NULL, 0};
    const uint8_t  *fb;

    dashboard_add_full_row(dash, &comp, 40);
    fb = dashboard_render_page(dash, 0);

    TEST_ASSERT_EQUAL_UINT8(0xFF, PX(fb, 0, 0, 0)); /* red at top */
    TEST_ASSERT_EQUAL_UINT8(0x00, PX(fb, 0, 0, 2));
    dashboard_destroy(dash);
}

static void test_render_page_1_shows_overflow_row_at_top(void)
{
    /* Row A (40px, red) on page 0; row B (40px, blue) overflows to page 1. */
    xf_dashboard_t *dash = dashboard_create(W, H, 0);
    xf_component_t  a    = {NULL, render_red, NULL, 0};
    xf_component_t  b    = {NULL, render_blue, NULL, 0};
    const uint8_t  *fb;

    dashboard_add_full_row(dash, &a, 40);
    dashboard_add_full_row(dash, &b, 40);
    fb = dashboard_render_page(dash, 1);

    /* Row B starts at y=0 on page 1. */
    TEST_ASSERT_EQUAL_UINT8(0x00, PX(fb, 0, 0, 0)); /* blue: R=0 */
    TEST_ASSERT_EQUAL_UINT8(0xFF, PX(fb, 0, 0, 2)); /* blue: B=255 */
    dashboard_destroy(dash);
}

static void test_render_page_0_excludes_overflow_row(void)
{
    /* After row A (40px), row B should not bleed into page 0 at y=40. */
    xf_dashboard_t *dash = dashboard_create(W, H, 0);
    xf_component_t  a    = {NULL, render_red, NULL, 0};
    xf_component_t  b    = {NULL, render_blue, NULL, 0};
    const uint8_t  *fb;

    dashboard_add_full_row(dash, &a, 40);
    dashboard_add_full_row(dash, &b, 40);
    fb = dashboard_render_page(dash, 0);

    /* y=40 must be background — row B is on page 1, not page 0. */
    TEST_ASSERT_EQUAL_UINT8(TO_R(xf_get_theme()->background), PX(fb, 0, 40, 0));
    TEST_ASSERT_EQUAL_UINT8(TO_B(xf_get_theme()->background), PX(fb, 0, 40, 2));
    dashboard_destroy(dash);
}

static void test_render_out_of_bounds_page_returns_background_buffer(void)
{
    xf_dashboard_t *dash = dashboard_create(W, H, 0);
    xf_component_t  comp = {NULL, render_red, NULL, 0};
    const uint8_t  *fb;

    dashboard_add_full_row(dash, &comp, 40);
    fb = dashboard_render_page(dash, 99); /* only 1 page exists */

    TEST_ASSERT_NOT_NULL(fb);
    TEST_ASSERT_EQUAL_UINT8(TO_R(xf_get_theme()->background), PX(fb, 0, 0, 0));
    TEST_ASSERT_EQUAL_UINT8(TO_G(xf_get_theme()->background), PX(fb, 0, 0, 1));
    TEST_ASSERT_EQUAL_UINT8(TO_B(xf_get_theme()->background), PX(fb, 0, 0, 2));
    dashboard_destroy(dash);
}

static void test_render_page_still_calls_fetch_and_render_callbacks(void)
{
    xf_dashboard_t *dash = dashboard_create(W, H, 0);
    xf_component_t  comp = {fetch_set_flag, render_check_flag, NULL, 0};

    dashboard_add_full_row(dash, &comp, 40);
    dashboard_render_page(dash, 0);

    TEST_ASSERT_TRUE(render_saw_fetch);
    dashboard_destroy(dash);
}

static void test_dashboard_render_is_equivalent_to_render_page_0(void)
{
    xf_dashboard_t *dash  = dashboard_create(W, H, 0);
    xf_component_t  a     = {NULL, render_red, NULL, 0};
    xf_component_t  b     = {NULL, render_blue, NULL, 0};
    const uint8_t  *fb0, *fb1;
    /* Copy page-0 result since the internal buffer is reused. */
    uint8_t page0_copy[W * H * 3];

    dashboard_add_full_row(dash, &a, 40);
    dashboard_add_full_row(dash, &b, 40); /* b overflows to page 1 */

    fb0 = dashboard_render_page(dash, 0);
    memcpy(page0_copy, fb0, sizeof(page0_copy));

    fb1 = dashboard_render(dash);
    TEST_ASSERT_EQUAL_MEMORY(page0_copy, fb1, sizeof(page0_copy));
    dashboard_destroy(dash);
}

/* ── dashboard_dirty_rect ────────────────────────────────────────────────── */

static void test_dirty_rect_returns_minus_one_for_null_dashboard(void)
{
    int x, y, w, h;
    TEST_ASSERT_EQUAL_INT(-1, dashboard_dirty_rect(NULL, 0, &x, &y, &w, &h));
}

static void test_dirty_rect_returns_zero_when_nothing_dirty(void)
{
    xf_dashboard_t *dash = dashboard_create(W, H, 0);
    xf_component_t  comp = {NULL, render_red, NULL, 0};
    int x, y, w, h;

    dashboard_add_full_row(dash, &comp, H);
    TEST_ASSERT_EQUAL_INT(0, dashboard_dirty_rect(dash, 0, &x, &y, &w, &h));
    dashboard_destroy(dash);
}

static void test_dirty_rect_returns_one_for_single_dirty_component(void)
{
    xf_dashboard_t *dash = dashboard_create(W, H, 0);
    xf_component_t  comp = {NULL, render_red, NULL, 1};
    int x, y, w, h;

    dashboard_add_full_row(dash, &comp, H);
    TEST_ASSERT_EQUAL_INT(1, dashboard_dirty_rect(dash, 0, &x, &y, &w, &h));
    TEST_ASSERT_EQUAL_INT(0, x);
    TEST_ASSERT_EQUAL_INT(0, y);
    TEST_ASSERT_EQUAL_INT(W, w);
    TEST_ASSERT_EQUAL_INT(H, h);
    dashboard_destroy(dash);
}

static void test_dirty_rect_clears_flag_after_call(void)
{
    xf_dashboard_t *dash = dashboard_create(W, H, 0);
    xf_component_t  comp = {NULL, render_red, NULL, 1};
    int x, y, w, h;

    dashboard_add_full_row(dash, &comp, H);
    dashboard_dirty_rect(dash, 0, &x, &y, &w, &h);
    TEST_ASSERT_EQUAL_INT(0, dashboard_dirty_rect(dash, 0, &x, &y, &w, &h));
    dashboard_destroy(dash);
}

static void test_dirty_rect_merges_two_dirty_components(void)
{
    /* Red (top, 20px) not dirty; blue (bottom, 40px) dirty.
     * Expected rect: x=0, y=20, w=W, h=40. */
    xf_dashboard_t *dash = dashboard_create(W, H, 0);
    xf_component_t  top  = {NULL, render_red,  NULL, 0};
    xf_component_t  bot  = {NULL, render_blue, NULL, 1};
    int x, y, w, h;

    dashboard_add_full_row(dash, &top, 20);
    dashboard_add_full_row(dash, &bot, 40);
    TEST_ASSERT_EQUAL_INT(1, dashboard_dirty_rect(dash, 0, &x, &y, &w, &h));
    TEST_ASSERT_EQUAL_INT(0,  x);
    TEST_ASSERT_EQUAL_INT(20, y);
    TEST_ASSERT_EQUAL_INT(W,  w);
    TEST_ASSERT_EQUAL_INT(40, h);
    dashboard_destroy(dash);
}

static void test_dirty_rect_union_covers_both_dirty_components(void)
{
    /* Red (20px) dirty, green (10px) not dirty, blue (30px) dirty.
     * Union rect must span from y=0 to y=59. */
    xf_dashboard_t *dash  = dashboard_create(W, H, 0);
    xf_component_t  top   = {NULL, render_red,   NULL, 1};
    xf_component_t  mid   = {NULL, render_green, NULL, 0};
    xf_component_t  bot   = {NULL, render_blue,  NULL, 1};
    int x, y, w, h;

    dashboard_add_full_row(dash, &top, 20);
    dashboard_add_full_row(dash, &mid, 10);
    dashboard_add_full_row(dash, &bot, 30);
    TEST_ASSERT_EQUAL_INT(1, dashboard_dirty_rect(dash, 0, &x, &y, &w, &h));
    TEST_ASSERT_EQUAL_INT(0,  x);
    TEST_ASSERT_EQUAL_INT(0,  y);
    TEST_ASSERT_EQUAL_INT(W,  w);
    TEST_ASSERT_EQUAL_INT(60, h); /* 20 + 10 + 30 */
    dashboard_destroy(dash);
}

static void test_dirty_rect_accounts_for_padding(void)
{
    /* 10px padding on all sides; single dirty component.
     * Component rect starts at (10,10) in framebuffer coords. */
    xf_dashboard_t *dash = dashboard_create(W, H, 10);
    xf_component_t  comp = {NULL, render_red, NULL, 1};
    int x, y, w, h;

    dashboard_add_full_row(dash, &comp, H - 20); /* content height */
    TEST_ASSERT_EQUAL_INT(1, dashboard_dirty_rect(dash, 0, &x, &y, &w, &h));
    TEST_ASSERT_EQUAL_INT(10,     x);
    TEST_ASSERT_EQUAL_INT(10,     y);
    TEST_ASSERT_EQUAL_INT(W - 20, w);
    TEST_ASSERT_EQUAL_INT(H - 20, h);
    dashboard_destroy(dash);
}

static void test_dirty_rect_ignores_components_on_other_page(void)
{
    /* Two 40px rows: first fits on page 0, second overflows to page 1.
     * Only the page-1 component is dirty; querying page 0 must return 0. */
    xf_dashboard_t *dash = dashboard_create(W, H, 0);
    xf_component_t  a    = {NULL, render_red,  NULL, 0};
    xf_component_t  b    = {NULL, render_blue, NULL, 1};
    int x, y, w, h;

    dashboard_add_full_row(dash, &a, 40);
    dashboard_add_full_row(dash, &b, 40); /* overflows to page 1 */
    TEST_ASSERT_EQUAL_INT(0, dashboard_dirty_rect(dash, 0, &x, &y, &w, &h));
    TEST_ASSERT_EQUAL_INT(1, dashboard_dirty_rect(dash, 1, &x, &y, &w, &h));
    dashboard_destroy(dash);
}

/* ── runner ──────────────────────────────────────────────────────────────── */

int main(void)
{
    UNITY_BEGIN();

    RUN_TEST(test_create_returns_non_null_for_valid_dimensions);
    RUN_TEST(test_destroy_null_is_a_noop);

    RUN_TEST(test_add_row_rejects_null_dashboard);
    RUN_TEST(test_add_row_rejects_null_components);
    RUN_TEST(test_add_row_rejects_null_widths);
    RUN_TEST(test_add_row_rejects_zero_count);
    RUN_TEST(test_add_row_rejects_zero_height);
    RUN_TEST(test_add_row_rejects_widths_that_dont_sum_to_display_width);

    RUN_TEST(test_add_full_row_rejects_null_dashboard);
    RUN_TEST(test_add_full_row_rejects_null_component);

    RUN_TEST(test_render_returns_null_for_null_dashboard);
    RUN_TEST(test_render_returns_non_null_for_valid_dashboard);

    RUN_TEST(test_render_calls_fetch_before_render);
    RUN_TEST(test_render_skips_fetch_when_null);
    RUN_TEST(test_render_proceeds_when_fetch_returns_error);

    RUN_TEST(test_render_passes_full_width_to_full_row_component);
    RUN_TEST(test_render_passes_half_width_to_split_row_component);

    RUN_TEST(test_single_full_row_pixels_appear_at_top_left);
    RUN_TEST(test_second_row_pixels_appear_below_first_row);
    RUN_TEST(test_right_column_pixels_appear_at_correct_x_offset);
    RUN_TEST(test_framebuffer_is_cleared_between_renders);

    RUN_TEST(test_move_row_up_swaps_visual_position);
    RUN_TEST(test_move_row_down_swaps_visual_position);
    RUN_TEST(test_move_row_up_on_first_row_returns_error);
    RUN_TEST(test_move_row_down_on_last_row_returns_error);
    RUN_TEST(test_remove_row_eliminates_pixels_from_next_render);
    RUN_TEST(test_remove_row_out_of_bounds_returns_error);

    RUN_TEST(test_page_count_returns_zero_for_null);
    RUN_TEST(test_page_count_is_one_for_empty_dashboard);
    RUN_TEST(test_page_count_is_one_when_rows_fit_within);
    RUN_TEST(test_page_count_is_one_when_rows_fit_exactly);
    RUN_TEST(test_page_count_is_two_when_rows_overflow);
    RUN_TEST(test_page_count_is_three_for_three_pages);

    RUN_TEST(test_render_page_returns_null_for_null_dashboard);
    RUN_TEST(test_render_page_0_shows_rows_fitting_display);
    RUN_TEST(test_render_page_1_shows_overflow_row_at_top);
    RUN_TEST(test_render_page_0_excludes_overflow_row);
    RUN_TEST(test_render_out_of_bounds_page_returns_background_buffer);
    RUN_TEST(test_render_page_still_calls_fetch_and_render_callbacks);
    RUN_TEST(test_dashboard_render_is_equivalent_to_render_page_0);

    RUN_TEST(test_dirty_rect_returns_minus_one_for_null_dashboard);
    RUN_TEST(test_dirty_rect_returns_zero_when_nothing_dirty);
    RUN_TEST(test_dirty_rect_returns_one_for_single_dirty_component);
    RUN_TEST(test_dirty_rect_clears_flag_after_call);
    RUN_TEST(test_dirty_rect_merges_two_dirty_components);
    RUN_TEST(test_dirty_rect_union_covers_both_dirty_components);
    RUN_TEST(test_dirty_rect_accounts_for_padding);
    RUN_TEST(test_dirty_rect_ignores_components_on_other_page);

    return UNITY_END();
}

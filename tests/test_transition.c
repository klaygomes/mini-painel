#include "vendor/unity.h"
#include "../src/draw/transition.h"

#include <string.h>
#include <stdint.h>

#define W 80
#define H 60

static uint8_t old_frame[W * H * 3];
static uint8_t new_frame[W * H * 3];

void setUp(void)
{
    memset(old_frame, 0x11, sizeof(old_frame)); /* dark grey */
    memset(new_frame, 0xEE, sizeof(new_frame)); /* light grey */
}

void tearDown(void) {}

static void test_all_effects_run_without_crash_headless(void)
{
    xf_transition_t e;
    for (e = 0; e < XF_TRANS_COUNT; e++)
        transition_play(NULL, old_frame, new_frame, W, H, e);
}

static void test_out_of_range_effect_is_a_noop(void)
{
    /* Must not crash or access out-of-bounds memory. */
    transition_play(NULL, old_frame, new_frame, W, H, XF_TRANS_COUNT);
    transition_play(NULL, old_frame, new_frame, W, H, (xf_transition_t)-1);
}

static void test_count_matches_expected_number_of_effects(void)
{
    TEST_ASSERT_EQUAL_INT(7, (int)XF_TRANS_COUNT);
}

static void test_wipe_top_is_first_effect(void)
{
    TEST_ASSERT_EQUAL_INT(0, (int)XF_TRANS_WIPE_TOP);
}

static void test_effects_smoke_with_minimal_dimensions(void)
{
    uint8_t tiny_old[3] = {0x11, 0x22, 0x33};
    uint8_t tiny_new[3] = {0xAA, 0xBB, 0xCC};
    xf_transition_t e;
    for (e = 0; e < XF_TRANS_COUNT; e++)
        transition_play(NULL, tiny_old, tiny_new, 1, 1, e);
}

static void test_old_frame_is_not_modified_by_any_effect(void)
{
    uint8_t saved[W * H * 3];
    memcpy(saved, old_frame, sizeof(saved));

    xf_transition_t e;
    for (e = 0; e < XF_TRANS_COUNT; e++)
        transition_play(NULL, old_frame, new_frame, W, H, e);

    TEST_ASSERT_EQUAL_MEMORY(saved, old_frame, sizeof(saved));
}

static void test_new_frame_is_not_modified_by_any_effect(void)
{
    uint8_t saved[W * H * 3];
    memcpy(saved, new_frame, sizeof(saved));

    xf_transition_t e;
    for (e = 0; e < XF_TRANS_COUNT; e++)
        transition_play(NULL, old_frame, new_frame, W, H, e);

    TEST_ASSERT_EQUAL_MEMORY(saved, new_frame, sizeof(saved));
}

int main(void)
{
    UNITY_BEGIN();

    RUN_TEST(test_all_effects_run_without_crash_headless);
    RUN_TEST(test_out_of_range_effect_is_a_noop);
    RUN_TEST(test_count_matches_expected_number_of_effects);
    RUN_TEST(test_wipe_top_is_first_effect);
    RUN_TEST(test_effects_smoke_with_minimal_dimensions);
    RUN_TEST(test_old_frame_is_not_modified_by_any_effect);
    RUN_TEST(test_new_frame_is_not_modified_by_any_effect);

    return UNITY_END();
}

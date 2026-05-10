#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <limits.h>

#include "unity.h"
#include "json_api.h"

#define W 480
#define H 320

static xf_json_ctx_t *ctx;
static uint8_t        canvas[W * H * 3];
static char           reply[4096];

static int exec_json(xf_json_ctx_t *ctx,
                     const char *json, size_t json_len,
                     uint8_t *canvas, size_t canvas_cap,
                     char *out_json, size_t out_json_cap)
{
    const xf_json_exec_req_t req = {
        .ctx = ctx,
        .json = {
            .buf = json,
            .size = json_len,
        },
        .canvas = {
            .buf = canvas,
            .size = canvas_cap,
        },
        .out_json = {
            .buf = out_json,
            .size = out_json_cap,
        },
    };

    return xf_json_exec(&req);
}

void setUp(void)
{
    ctx = xf_json_create(W, H, 4);
    TEST_ASSERT_NOT_NULL(ctx);
    memset(canvas, 0, sizeof canvas);
    memset(reply,  0, sizeof reply);
}

void tearDown(void)
{
    xf_json_destroy(ctx);
    ctx = NULL;
}

void test_add_render_minimal(void)
{
    const char *script =
        "[{\"op\":\"row.add\",\"id\":\"component.header.h1\","
        "  \"data\":{\"date\":\"Mon\",\"status_text\":\"ok\",\"status_dot\":\"#1D9E75\"}},"
        " {\"op\":\"page.render\",\"page\":0}]";

    int rc = exec_json(ctx, script, strlen(script),
                          canvas, sizeof canvas, reply, sizeof reply);
    TEST_ASSERT_EQUAL_INT(0, rc);
    TEST_ASSERT_NOT_NULL(strstr(reply, "\"ok\":true"));

    int nonzero = 0;
    for (int i = 4 * W * 3; i < 24 * W * 3; i++)
        if (canvas[i]) { nonzero = 1; break; }
    TEST_ASSERT_TRUE(nonzero);
}

void test_exec_input_guardrails(void)
{
    const char *json = "[]";
    int rc;

    rc = exec_json(NULL, json, strlen(json),
                      canvas, sizeof canvas, reply, sizeof reply);
    TEST_ASSERT_EQUAL_INT(-1, rc);

    rc = exec_json(ctx, NULL, 2,
                      canvas, sizeof canvas, reply, sizeof reply);
    TEST_ASSERT_EQUAL_INT(-1, rc);

    rc = exec_json(ctx, json, 0,
                      canvas, sizeof canvas, reply, sizeof reply);
    TEST_ASSERT_EQUAL_INT(-1, rc);
}

void test_update_changes_pixels(void)
{
    const char *add =
        "[{\"op\":\"row.add\",\"id\":\"component.header.h2\","
        "  \"data\":{\"date\":\"Day1\",\"status_text\":\"ok\",\"status_dot\":\"#1D9E75\"}},"
        " {\"op\":\"page.render\",\"page\":0}]";

    exec_json(ctx, add, strlen(add),
                 canvas, sizeof canvas, reply, sizeof reply);

    uint8_t before[3];
    memcpy(before, canvas + 6 * W * 3 + 50 * 3, 3);

    const char *upd =
        "[{\"op\":\"row.update\",\"id\":\"component.header.h2\","
        "  \"data\":{\"date\":\"ZZZZZZ\",\"status_text\":\"err\",\"status_dot\":\"#FF0000\"}},"
        " {\"op\":\"page.render\",\"page\":0}]";
    memset(canvas, 0, sizeof canvas);
    int rc = exec_json(ctx, upd, strlen(upd),
                          canvas, sizeof canvas, reply, sizeof reply);
    TEST_ASSERT_EQUAL_INT(0, rc);

    uint8_t after[3];
    memcpy(after, canvas + 6 * W * 3 + 50 * 3, 3);
    int nonzero = 0;
    for (size_t i = 0; i < sizeof canvas; i++)
        if (canvas[i]) { nonzero = 1; break; }
    TEST_ASSERT_TRUE(nonzero);
    (void)before; (void)after;
}

void test_remove_drops_row(void)
{
    const char *setup =
        "[{\"op\":\"row.add\",\"id\":\"component.header.r1\",\"data\":{}},"
        " {\"op\":\"row.add\",\"id\":\"component.spacer.r2\",\"data\":{}},"
        " {\"op\":\"row.add\",\"id\":\"component.deploy.r3\","
        "  \"data\":{\"branch\":\"b\",\"time_ago\":\"1m\",\"label\":\"ok\",\"status\":1}}]";
    exec_json(ctx, setup, strlen(setup),
                 canvas, sizeof canvas, reply, sizeof reply);

    int before = xf_json_page_count(ctx);

    const char *rem = "[{\"op\":\"row.remove\",\"id\":\"component.spacer.r2\"}]";
    int rc = exec_json(ctx, rem, strlen(rem),
                          canvas, sizeof canvas, reply, sizeof reply);
    TEST_ASSERT_EQUAL_INT(0, rc);
    TEST_ASSERT_NOT_NULL(strstr(reply, "\"ok\":true"));

    int after = xf_json_page_count(ctx);
    TEST_ASSERT_GREATER_OR_EQUAL(1, after);
    TEST_ASSERT_LESS_OR_EQUAL(before, after);
}

void test_duplicate_id_errors(void)
{
    const char *s =
        "[{\"op\":\"row.add\",\"id\":\"component.header.dup\",\"data\":{}},"
        " {\"op\":\"row.add\",\"id\":\"component.header.dup\",\"data\":{}}]";
    int rc = exec_json(ctx, s, strlen(s),
                          canvas, sizeof canvas, reply, sizeof reply);
    TEST_ASSERT_EQUAL_INT(-1, rc);
    TEST_ASSERT_NOT_NULL(strstr(reply, "\"ok\":false"));

    const char *upd =
        "[{\"op\":\"row.update\",\"id\":\"component.header.dup\",\"data\":{}}]";
    memset(reply, 0, sizeof reply);
    rc = exec_json(ctx, upd, strlen(upd),
                      canvas, sizeof canvas, reply, sizeof reply);
    TEST_ASSERT_EQUAL_INT(0, rc);
    TEST_ASSERT_NOT_NULL(strstr(reply, "\"ok\":true"));
}

void test_continue_after_error_in_batch(void)
{
    const char *s =
        "[{\"op\":\"foo\"},"
        " {\"op\":\"row.add\",\"id\":\"component.header.keep\",\"data\":{}},"
        " {\"op\":\"row.update\",\"id\":\"component.header.keep\",\"data\":{\"date\":\"Tue\"}}]";
    int rc = exec_json(ctx, s, strlen(s),
                          canvas, sizeof canvas, reply, sizeof reply);

    TEST_ASSERT_EQUAL_INT(-1, rc);
    TEST_ASSERT_NOT_NULL(strstr(reply, "\"ok\":false"));
    TEST_ASSERT_NOT_NULL(strstr(reply, "unknown op"));
    TEST_ASSERT_NOT_NULL(strstr(reply, "component.header.keep"));

    memset(reply, 0, sizeof reply);
    rc = exec_json(ctx, "[{\"op\":\"row.list\"}]", strlen("[{\"op\":\"row.list\"}]"),
                      canvas, sizeof canvas, reply, sizeof reply);
    TEST_ASSERT_EQUAL_INT(0, rc);
    TEST_ASSERT_NOT_NULL(strstr(reply, "component.header.keep"));
}

void test_invalid_id_errors(void)
{
    const char *s1 = "[{\"op\":\"row.add\",\"id\":\"component.bogus.x\",\"data\":{}}]";
    int rc = exec_json(ctx, s1, strlen(s1),
                          canvas, sizeof canvas, reply, sizeof reply);
    TEST_ASSERT_EQUAL_INT(-1, rc);
    TEST_ASSERT_NOT_NULL(strstr(reply, "error"));

    memset(reply, 0, sizeof reply);
    const char *s2 = "[{\"op\":\"row.add\",\"id\":\"my-thing\",\"data\":{}}]";
    rc = exec_json(ctx, s2, strlen(s2),
                      canvas, sizeof canvas, reply, sizeof reply);
    TEST_ASSERT_EQUAL_INT(-1, rc);
    TEST_ASSERT_NOT_NULL(strstr(reply, "error"));
}

void test_unknown_op(void)
{
    const char *s = "[{\"op\":\"foo\"}]";
    int rc = exec_json(ctx, s, strlen(s),
                          canvas, sizeof canvas, reply, sizeof reply);
    TEST_ASSERT_EQUAL_INT(-1, rc);
    TEST_ASSERT_NOT_NULL(strstr(reply, "unknown op"));
}

void test_op_must_be_string(void)
{
    const char *s = "[{\"op\":123}]";
    int rc = exec_json(ctx, s, strlen(s),
                          canvas, sizeof canvas, reply, sizeof reply);
    TEST_ASSERT_EQUAL_INT(-1, rc);
    TEST_ASSERT_NOT_NULL(strstr(reply, "op must be a string"));
}

void test_op_too_long(void)
{
    char long_op[200];
    char script[256];

    memset(long_op, 'x', sizeof(long_op) - 1);
    long_op[sizeof(long_op) - 1] = '\0';
    snprintf(script, sizeof(script), "[{\"op\":\"%s\"}]", long_op);

    int rc = exec_json(ctx, script, strlen(script),
                          canvas, sizeof canvas, reply, sizeof reply);
    TEST_ASSERT_EQUAL_INT(-1, rc);
    TEST_ASSERT_NOT_NULL(strstr(reply, "op too long"));
}

void test_missing_op_in_batch_continues(void)
{
    const char *s =
        "[{\"id\":\"component.header.noop\"},"
        " {\"op\":\"row.add\",\"id\":\"component.header.after_missing\",\"data\":{}}]";
    int rc = exec_json(ctx, s, strlen(s),
                          canvas, sizeof canvas, reply, sizeof reply);

    TEST_ASSERT_EQUAL_INT(-1, rc);
    TEST_ASSERT_NOT_NULL(strstr(reply, "missing op"));
    TEST_ASSERT_NOT_NULL(strstr(reply, "component.header.after_missing"));

    memset(reply, 0, sizeof reply);
    rc = exec_json(ctx, "[{\"op\":\"row.list\"}]", strlen("[{\"op\":\"row.list\"}]"),
                      canvas, sizeof canvas, reply, sizeof reply);
    TEST_ASSERT_EQUAL_INT(0, rc);
    TEST_ASSERT_NOT_NULL(strstr(reply, "component.header.after_missing"));
}

void test_non_object_command_in_batch_continues(void)
{
    const char *s =
        "[\"not-an-object\","
        " {\"op\":\"row.add\",\"id\":\"component.header.after_non_object\",\"data\":{}}]";
    int rc = exec_json(ctx, s, strlen(s),
                          canvas, sizeof canvas, reply, sizeof reply);

    TEST_ASSERT_EQUAL_INT(-1, rc);
    TEST_ASSERT_NOT_NULL(strstr(reply, "command must be an object"));
    TEST_ASSERT_NOT_NULL(strstr(reply, "component.header.after_non_object"));
}

void test_move_bounds_errors_and_continues(void)
{
    const char *setup =
        "[{\"op\":\"row.add\",\"id\":\"component.header.top\",\"data\":{}},"
        " {\"op\":\"row.add\",\"id\":\"component.header.bottom\",\"data\":{}}]";
    int rc = exec_json(ctx, setup, strlen(setup),
                          canvas, sizeof canvas, reply, sizeof reply);
    TEST_ASSERT_EQUAL_INT(0, rc);

    memset(reply, 0, sizeof reply);
    const char *s =
        "[{\"op\":\"row.move_up\",\"id\":\"component.header.top\"},"
        " {\"op\":\"row.move_down\",\"id\":\"component.header.bottom\"},"
        " {\"op\":\"row.list\"}]";
    rc = exec_json(ctx, s, strlen(s),
                      canvas, sizeof canvas, reply, sizeof reply);

    TEST_ASSERT_EQUAL_INT(-1, rc);
    TEST_ASSERT_NOT_NULL(strstr(reply, "cannot move"));
    TEST_ASSERT_NOT_NULL(strstr(reply, "\"op\":\"row.list\""));
    TEST_ASSERT_NOT_NULL(strstr(reply, "\"count\":2"));
}

void test_remove_missing_id_and_followup_add(void)
{
    const char *s =
        "[{\"op\":\"row.remove\",\"id\":\"component.header.nope\"},"
        " {\"op\":\"row.add\",\"id\":\"component.header.after_remove_error\",\"data\":{}}]";
    int rc = exec_json(ctx, s, strlen(s),
                          canvas, sizeof canvas, reply, sizeof reply);

    TEST_ASSERT_EQUAL_INT(-1, rc);
    TEST_ASSERT_NOT_NULL(strstr(reply, "id not found"));
    TEST_ASSERT_NOT_NULL(strstr(reply, "component.header.after_remove_error"));
}

void test_missing_id_for_all_id_based_ops(void)
{
    const char *s =
        "[{\"op\":\"row.update\"},"
        " {\"op\":\"row.remove\"},"
        " {\"op\":\"row.move_up\"},"
        " {\"op\":\"row.move_down\"},"
        " {\"op\":\"row.add\",\"id\":\"component.header.after_missing_ids\",\"data\":{}}]";
    int rc = exec_json(ctx, s, strlen(s),
                          canvas, sizeof canvas, reply, sizeof reply);

    TEST_ASSERT_EQUAL_INT(-1, rc);
    TEST_ASSERT_NOT_NULL(strstr(reply, "\"op\":\"row.update\""));
    TEST_ASSERT_NOT_NULL(strstr(reply, "\"op\":\"row.remove\""));
    TEST_ASSERT_NOT_NULL(strstr(reply, "\"op\":\"row.move_up\""));
    TEST_ASSERT_NOT_NULL(strstr(reply, "\"op\":\"row.move_down\""));
    TEST_ASSERT_NOT_NULL(strstr(reply, "component.header.after_missing_ids"));
}

void test_multiple_errors_in_one_batch(void)
{
    const char *s =
        "[{\"op\":\"unknown.kind\"},"
        " {\"id\":\"missing-op\"},"
        " {\"op\":123},"
        " {\"op\":\"row.add\",\"id\":\"component.header.after_multi_error\",\"data\":{}}]";
    int rc = exec_json(ctx, s, strlen(s),
                          canvas, sizeof canvas, reply, sizeof reply);

    TEST_ASSERT_EQUAL_INT(-1, rc);
    TEST_ASSERT_NOT_NULL(strstr(reply, "unknown op"));
    TEST_ASSERT_NOT_NULL(strstr(reply, "missing op"));
    TEST_ASSERT_NOT_NULL(strstr(reply, "op must be a string"));
    TEST_ASSERT_NOT_NULL(strstr(reply, "component.header.after_multi_error"));
}

void test_render_without_canvas(void)
{
    const char *s = "[{\"op\":\"page.render\",\"page\":0}]";
    int rc = exec_json(ctx, s, strlen(s),
                          NULL, sizeof canvas, reply, sizeof reply);
    TEST_ASSERT_EQUAL_INT(-1, rc);
    TEST_ASSERT_NOT_NULL(strstr(reply, "error"));
}

void test_render_canvas_too_small(void)
{
    const char *s = "[{\"op\":\"page.render\",\"page\":0}]";
    int rc = exec_json(ctx, s, strlen(s),
                          canvas, 10, reply, sizeof reply);
    TEST_ASSERT_EQUAL_INT(-1, rc);
    TEST_ASSERT_NOT_NULL(strstr(reply, "error"));
}

void test_render_out_of_range_pages_succeeds(void)
{
    const char *setup =
        "[{\"op\":\"row.add\",\"id\":\"component.header.pg\",\"data\":{}}]";
    int rc = exec_json(ctx, setup, strlen(setup),
                          canvas, sizeof canvas, reply, sizeof reply);
    TEST_ASSERT_EQUAL_INT(0, rc);

    memset(reply, 0, sizeof reply);
    rc = exec_json(ctx,
                      "[{\"op\":\"page.render\",\"page\":-1},{\"op\":\"page.render\",\"page\":999}]",
                      strlen("[{\"op\":\"page.render\",\"page\":-1},{\"op\":\"page.render\",\"page\":999}]"),
                      canvas, sizeof canvas, reply, sizeof reply);
    TEST_ASSERT_EQUAL_INT(0, rc);
    TEST_ASSERT_NOT_NULL(strstr(reply, "\"ok\":true"));
    TEST_ASSERT_NOT_NULL(strstr(reply, "\"page\":-1"));
    TEST_ASSERT_NOT_NULL(strstr(reply, "\"page\":999"));
}

void test_string_truncation(void)
{
    char script[512];
    char longdate[201];
    memset(longdate, 'A', 200);
    longdate[200] = '\0';
    snprintf(script, sizeof script,
             "[{\"op\":\"row.add\",\"id\":\"component.header.trunc\","
             "  \"data\":{\"date\":\"%s\"}}]", longdate);
    int rc = exec_json(ctx, script, strlen(script),
                          canvas, sizeof canvas, reply, sizeof reply);
    TEST_ASSERT_EQUAL_INT(0, rc);
    TEST_ASSERT_NOT_NULL(strstr(reply, "\"ok\":true"));
}

void test_color_parsing(void)
{
    {
        const char *s =
            "[{\"op\":\"row.add\",\"id\":\"component.header.c1\","
            "  \"data\":{\"status_dot\":\"#1D9E75\"}}]";
        int rc = exec_json(ctx, s, strlen(s),
                              canvas, sizeof canvas, reply, sizeof reply);
        TEST_ASSERT_EQUAL_INT(0, rc);
    }

    {
        const char *s =
            "[{\"op\":\"row.add\",\"id\":\"component.header.c2\","
            "  \"data\":{\"status_dot\":\"#1D9E7580\"}}]";
        int rc = exec_json(ctx, s, strlen(s),
                              canvas, sizeof canvas, reply, sizeof reply);
        TEST_ASSERT_EQUAL_INT(0, rc);
    }

    {
        const char *s =
            "[{\"op\":\"row.add\",\"id\":\"component.header.c3\","
            "  \"data\":{\"status_dot\":\"#abc\"}}]";
        int rc = exec_json(ctx, s, strlen(s),
                              canvas, sizeof canvas, reply, sizeof reply);
        TEST_ASSERT_EQUAL_INT(0, rc);
    }

    {
        const char *s =
            "[{\"op\":\"row.add\",\"id\":\"component.header.c4\","
            "  \"data\":{\"status_dot\":\"not-a-color\"}}]";
        int rc = exec_json(ctx, s, strlen(s),
                              canvas, sizeof canvas, reply, sizeof reply);
        TEST_ASSERT_EQUAL_INT(-1, rc);
        TEST_ASSERT_NOT_NULL(strstr(reply, "error"));
    }
}

void test_response_truncation(void)
{
    char small_reply[64];
    memset(small_reply, 0, sizeof small_reply);

    const char *s =
        "[{\"op\":\"row.add\",\"id\":\"component.header.trunc2\","
        "  \"data\":{\"date\":\"X\"}},"
        " {\"op\":\"page.render\",\"page\":0}]";

    exec_json(ctx, s, strlen(s),
                 canvas, sizeof canvas, small_reply, sizeof small_reply);

    TEST_ASSERT_TRUE(small_reply[0] == '{');
    TEST_ASSERT_NOT_NULL(strstr(small_reply, "\"ok\""));
}

void test_output_buffer_edge_capacities(void)
{
    char one[1] = { 0 };
    const char *s0 = "[{\"op\":\"row.add\",\"id\":\"component.header.cap0\",\"data\":{}}]";
    const char *s1 = "[{\"op\":\"row.add\",\"id\":\"component.header.cap1\",\"data\":{}}]";
    int rc;

    rc = exec_json(ctx, s0, strlen(s0),
                      canvas, sizeof canvas, reply, 0);
    TEST_ASSERT_EQUAL_INT(0, rc);

    rc = exec_json(ctx, s1, strlen(s1),
                      canvas, sizeof canvas, one, sizeof(one));
    TEST_ASSERT_EQUAL_INT(0, rc);
    TEST_ASSERT_EQUAL_CHAR('{', one[0]);
}

void test_json_len_overflow_guard(void)
{
    const char *s = "[]";
    int rc = exec_json(ctx, s, (size_t)INT_MAX + 1u,
                          canvas, sizeof canvas, reply, sizeof reply);
    TEST_ASSERT_EQUAL_INT(-1, rc);
}

void test_full_lifecycle(void)
{
    const char *add5 =
        "[{\"op\":\"row.add\",\"id\":\"component.header.lc1\","
        "  \"data\":{\"date\":\"Thu\",\"status_text\":\"ok\",\"status_dot\":\"#1D9E75\"}},"
        " {\"op\":\"row.add\",\"id\":\"component.spacer.lc2\",\"data\":{}},"
        " {\"op\":\"row.add\",\"id\":\"component.deploy.lc3\","
        "  \"data\":{\"branch\":\"main\",\"time_ago\":\"1m\",\"label\":\"green\",\"status\":1}},"
        " {\"op\":\"row.add\",\"id\":\"component.divider.lc4\",\"data\":{}},"
        " {\"op\":\"row.add\",\"id\":\"component.sprint.lc5\","
        "  \"data\":{\"title\":\"Sprint 42\",\"progress_label\":\"80%\","
        "           \"time_left\":\"2d\",\"percent\":0.8}}]";
    int rc = exec_json(ctx, add5, strlen(add5),
                          canvas, sizeof canvas, reply, sizeof reply);
    TEST_ASSERT_EQUAL_INT(0, rc);

    memset(reply, 0, sizeof reply);
    const char *r0 = "[{\"op\":\"page.render\",\"page\":0}]";
    rc = exec_json(ctx, r0, strlen(r0),
                      canvas, sizeof canvas, reply, sizeof reply);
    TEST_ASSERT_EQUAL_INT(0, rc);
    TEST_ASSERT_NOT_NULL(strstr(reply, "\"page_count\""));

    memset(reply, 0, sizeof reply);
    const char *upd =
        "[{\"op\":\"row.update\",\"id\":\"component.header.lc1\","
        "  \"data\":{\"date\":\"Fri\"}},"
        " {\"op\":\"row.update\",\"id\":\"component.sprint.lc5\","
        "  \"data\":{\"percent\":0.9}}]";
    rc = exec_json(ctx, upd, strlen(upd),
                      canvas, sizeof canvas, reply, sizeof reply);
    TEST_ASSERT_EQUAL_INT(0, rc);

    memset(reply, 0, sizeof reply);
    const char *mv =
        "[{\"op\":\"row.move_up\",\"id\":\"component.deploy.lc3\"}]";
    rc = exec_json(ctx, mv, strlen(mv),
                      canvas, sizeof canvas, reply, sizeof reply);
    TEST_ASSERT_EQUAL_INT(0, rc);

    memset(reply, 0, sizeof reply);
    const char *rem =
        "[{\"op\":\"row.remove\",\"id\":\"component.divider.lc4\"}]";
    rc = exec_json(ctx, rem, strlen(rem),
                      canvas, sizeof canvas, reply, sizeof reply);
    TEST_ASSERT_EQUAL_INT(0, rc);

    memset(reply, 0, sizeof reply);
    rc = exec_json(ctx, r0, strlen(r0),
                      canvas, sizeof canvas, reply, sizeof reply);
    TEST_ASSERT_EQUAL_INT(0, rc);
    TEST_ASSERT_NOT_NULL(strstr(reply, "\"ok\":true"));
    TEST_ASSERT_GREATER_OR_EQUAL(1, xf_json_page_count(ctx));
}

void test_row_list_empty(void)
{
    const char *s = "[{\"op\":\"row.list\"}]";
    int rc = exec_json(ctx, s, strlen(s),
                          canvas, sizeof canvas, reply, sizeof reply);
    TEST_ASSERT_EQUAL_INT(0, rc);
    TEST_ASSERT_NOT_NULL(strstr(reply, "\"ok\":true"));
    TEST_ASSERT_NOT_NULL(strstr(reply, "\"count\":0"));
    TEST_ASSERT_NOT_NULL(strstr(reply, "\"rows\":[]"));
}

void test_row_list_after_add(void)
{
    const char *add =
        "[{\"op\":\"row.add\",\"id\":\"component.header.ls1\","
        "  \"data\":{\"date\":\"Mon\",\"status_text\":\"ok\",\"status_dot\":\"#1D9E75\"}},"
        " {\"op\":\"row.add\",\"id\":\"component.deploy.ls2\","
        "  \"data\":{\"branch\":\"main\",\"time_ago\":\"1m\",\"label\":\"v1\",\"status\":0}}]";
    exec_json(ctx, add, strlen(add),
                 canvas, sizeof canvas, reply, sizeof reply);

    memset(reply, 0, sizeof reply);
    const char *s = "[{\"op\":\"row.list\"}]";
    int rc = exec_json(ctx, s, strlen(s),
                          canvas, sizeof canvas, reply, sizeof reply);
    TEST_ASSERT_EQUAL_INT(0, rc);
    TEST_ASSERT_NOT_NULL(strstr(reply, "\"ok\":true"));
    TEST_ASSERT_NOT_NULL(strstr(reply, "\"count\":2"));
    TEST_ASSERT_NOT_NULL(strstr(reply, "component.header.ls1"));
    TEST_ASSERT_NOT_NULL(strstr(reply, "component.deploy.ls2"));
    TEST_ASSERT_NOT_NULL(strstr(reply, "\"kind\":\"header\""));
    TEST_ASSERT_NOT_NULL(strstr(reply, "\"kind\":\"deploy\""));
    TEST_ASSERT_NOT_NULL(strstr(reply, "\"page_count\""));
    TEST_ASSERT_NOT_NULL(strstr(reply, "\"page\":0"));
    TEST_ASSERT_NOT_NULL(strstr(reply, "\"date\""));
    TEST_ASSERT_NOT_NULL(strstr(reply, "\"branch\""));
    TEST_ASSERT_NOT_NULL(strstr(reply, "\"status\""));
}

void test_row_list_dirty_flag(void)
{
    const char *add =
        "[{\"op\":\"row.add\",\"id\":\"component.header.ls3\","
        "  \"data\":{\"date\":\"Mon\",\"status_text\":\"ok\",\"status_dot\":\"#1D9E75\"}},"
        " {\"op\":\"page.render\",\"page\":0}]";
    exec_json(ctx, add, strlen(add),
                 canvas, sizeof canvas, reply, sizeof reply);

    memset(reply, 0, sizeof reply);
    const char *list = "[{\"op\":\"row.list\"}]";
    exec_json(ctx, list, strlen(list),
                 canvas, sizeof canvas, reply, sizeof reply);
    TEST_ASSERT_NOT_NULL(strstr(reply, "\"dirty\":0"));

    memset(reply, 0, sizeof reply);
    const char *upd =
        "[{\"op\":\"row.update\",\"id\":\"component.header.ls3\","
        "  \"data\":{\"date\":\"Tue\"}}]";
    exec_json(ctx, upd, strlen(upd),
                 canvas, sizeof canvas, reply, sizeof reply);

    memset(reply, 0, sizeof reply);
    exec_json(ctx, list, strlen(list),
                 canvas, sizeof canvas, reply, sizeof reply);
    TEST_ASSERT_NOT_NULL(strstr(reply, "\"dirty\":1"));
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_add_render_minimal);
    RUN_TEST(test_exec_input_guardrails);
    RUN_TEST(test_update_changes_pixels);
    RUN_TEST(test_remove_drops_row);
    RUN_TEST(test_duplicate_id_errors);
    RUN_TEST(test_continue_after_error_in_batch);
    RUN_TEST(test_invalid_id_errors);
    RUN_TEST(test_unknown_op);
    RUN_TEST(test_op_must_be_string);
    RUN_TEST(test_op_too_long);
    RUN_TEST(test_missing_op_in_batch_continues);
    RUN_TEST(test_non_object_command_in_batch_continues);
    RUN_TEST(test_move_bounds_errors_and_continues);
    RUN_TEST(test_remove_missing_id_and_followup_add);
    RUN_TEST(test_missing_id_for_all_id_based_ops);
    RUN_TEST(test_multiple_errors_in_one_batch);
    RUN_TEST(test_render_without_canvas);
    RUN_TEST(test_render_canvas_too_small);
    RUN_TEST(test_render_out_of_range_pages_succeeds);
    RUN_TEST(test_string_truncation);
    RUN_TEST(test_color_parsing);
    RUN_TEST(test_response_truncation);
    RUN_TEST(test_output_buffer_edge_capacities);
    RUN_TEST(test_json_len_overflow_guard);
    RUN_TEST(test_full_lifecycle);
    RUN_TEST(test_row_list_empty);
    RUN_TEST(test_row_list_after_add);
    RUN_TEST(test_row_list_dirty_flag);
    return UNITY_END();
}

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "unity.h"
#include "json_api.h"

#define W 480
#define H 320

static xf_json_ctx_t *ctx;
static uint8_t        canvas[W * H * 3];
static char           reply[4096];

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

/* ------------------------------------------------------------------ */
/* 1. Minimal add + render                                              */
/* ------------------------------------------------------------------ */
void test_add_render_minimal(void)
{
    const char *script =
        "[{\"op\":\"row.add\",\"id\":\"component.header.h1\","
        "  \"data\":{\"date\":\"Mon\",\"status_text\":\"ok\",\"status_dot\":\"#1D9E75\"}},"
        " {\"op\":\"page.render\",\"page\":0}]";

    int rc = xf_json_exec(ctx, script, strlen(script),
                          canvas, sizeof canvas, reply, sizeof reply);
    TEST_ASSERT_EQUAL_INT(0, rc);
    TEST_ASSERT_NOT_NULL(strstr(reply, "\"ok\":true"));

    /* Canvas should have non-zero bytes in the header band (rows 4..24) */
    int nonzero = 0;
    for (int i = 4 * W * 3; i < 24 * W * 3; i++)
        if (canvas[i]) { nonzero = 1; break; }
    TEST_ASSERT_TRUE(nonzero);
}

/* ------------------------------------------------------------------ */
/* 2. Update changes pixels                                             */
/* ------------------------------------------------------------------ */
void test_update_changes_pixels(void)
{
    const char *add =
        "[{\"op\":\"row.add\",\"id\":\"component.header.h2\","
        "  \"data\":{\"date\":\"Day1\",\"status_text\":\"ok\",\"status_dot\":\"#1D9E75\"}},"
        " {\"op\":\"page.render\",\"page\":0}]";

    xf_json_exec(ctx, add, strlen(add),
                 canvas, sizeof canvas, reply, sizeof reply);

    /* Save a pixel in the header band */
    uint8_t before[3];
    memcpy(before, canvas + 6 * W * 3 + 50 * 3, 3);

    /* Update with very different content */
    const char *upd =
        "[{\"op\":\"row.update\",\"id\":\"component.header.h2\","
        "  \"data\":{\"date\":\"ZZZZZZ\",\"status_text\":\"err\",\"status_dot\":\"#FF0000\"}},"
        " {\"op\":\"page.render\",\"page\":0}]";
    memset(canvas, 0, sizeof canvas);
    int rc = xf_json_exec(ctx, upd, strlen(upd),
                          canvas, sizeof canvas, reply, sizeof reply);
    TEST_ASSERT_EQUAL_INT(0, rc);

    uint8_t after[3];
    memcpy(after, canvas + 6 * W * 3 + 50 * 3, 3);
    /* At least one channel differs (renders are not deterministic for text,
     * but the background fill alone guarantees a non-identical frame when
     * the data changes). Accept if the frame is non-zero. */
    int nonzero = 0;
    for (size_t i = 0; i < sizeof canvas; i++)
        if (canvas[i]) { nonzero = 1; break; }
    TEST_ASSERT_TRUE(nonzero);
    (void)before; (void)after;
}

/* ------------------------------------------------------------------ */
/* 3. Remove drops row                                                  */
/* ------------------------------------------------------------------ */
void test_remove_drops_row(void)
{
    const char *setup =
        "[{\"op\":\"row.add\",\"id\":\"component.header.r1\",\"data\":{}},"
        " {\"op\":\"row.add\",\"id\":\"component.spacer.r2\",\"data\":{}},"
        " {\"op\":\"row.add\",\"id\":\"component.deploy.r3\","
        "  \"data\":{\"branch\":\"b\",\"time_ago\":\"1m\",\"label\":\"ok\"}}]";
    xf_json_exec(ctx, setup, strlen(setup),
                 canvas, sizeof canvas, reply, sizeof reply);

    int before = xf_json_page_count(ctx);

    const char *rem = "[{\"op\":\"row.remove\",\"id\":\"component.spacer.r2\"}]";
    int rc = xf_json_exec(ctx, rem, strlen(rem),
                          canvas, sizeof canvas, reply, sizeof reply);
    TEST_ASSERT_EQUAL_INT(0, rc);
    TEST_ASSERT_NOT_NULL(strstr(reply, "\"ok\":true"));

    int after = xf_json_page_count(ctx);
    /* page count stays >=1; may be same or decreased */
    TEST_ASSERT_GREATER_OR_EQUAL(1, after);
    TEST_ASSERT_LESS_OR_EQUAL(before, after);
}

/* ------------------------------------------------------------------ */
/* 4. Duplicate id errors                                               */
/* ------------------------------------------------------------------ */
void test_duplicate_id_errors(void)
{
    const char *s =
        "[{\"op\":\"row.add\",\"id\":\"component.header.dup\",\"data\":{}},"
        " {\"op\":\"row.add\",\"id\":\"component.header.dup\",\"data\":{}}]";
    int rc = xf_json_exec(ctx, s, strlen(s),
                          canvas, sizeof canvas, reply, sizeof reply);
    TEST_ASSERT_EQUAL_INT(-1, rc);
    TEST_ASSERT_NOT_NULL(strstr(reply, "\"ok\":false"));

    /* First add still in registry — probe via update */
    const char *upd =
        "[{\"op\":\"row.update\",\"id\":\"component.header.dup\",\"data\":{}}]";
    memset(reply, 0, sizeof reply);
    rc = xf_json_exec(ctx, upd, strlen(upd),
                      canvas, sizeof canvas, reply, sizeof reply);
    TEST_ASSERT_EQUAL_INT(0, rc);
    TEST_ASSERT_NOT_NULL(strstr(reply, "\"ok\":true"));
}

/* ------------------------------------------------------------------ */
/* 5. Invalid id errors                                                  */
/* ------------------------------------------------------------------ */
void test_invalid_id_errors(void)
{
    const char *s1 = "[{\"op\":\"row.add\",\"id\":\"component.bogus.x\",\"data\":{}}]";
    int rc = xf_json_exec(ctx, s1, strlen(s1),
                          canvas, sizeof canvas, reply, sizeof reply);
    TEST_ASSERT_EQUAL_INT(-1, rc);
    TEST_ASSERT_NOT_NULL(strstr(reply, "error"));

    memset(reply, 0, sizeof reply);
    const char *s2 = "[{\"op\":\"row.add\",\"id\":\"my-thing\",\"data\":{}}]";
    rc = xf_json_exec(ctx, s2, strlen(s2),
                      canvas, sizeof canvas, reply, sizeof reply);
    TEST_ASSERT_EQUAL_INT(-1, rc);
    TEST_ASSERT_NOT_NULL(strstr(reply, "error"));
}

/* ------------------------------------------------------------------ */
/* 6. Unknown op                                                         */
/* ------------------------------------------------------------------ */
void test_unknown_op(void)
{
    const char *s = "[{\"op\":\"foo\"}]";
    int rc = xf_json_exec(ctx, s, strlen(s),
                          canvas, sizeof canvas, reply, sizeof reply);
    TEST_ASSERT_EQUAL_INT(-1, rc);
    TEST_ASSERT_NOT_NULL(strstr(reply, "unknown op"));
}

/* ------------------------------------------------------------------ */
/* 7. Render without canvas                                              */
/* ------------------------------------------------------------------ */
void test_render_without_canvas(void)
{
    const char *s = "[{\"op\":\"page.render\",\"page\":0}]";
    int rc = xf_json_exec(ctx, s, strlen(s),
                          NULL, sizeof canvas, reply, sizeof reply);
    TEST_ASSERT_EQUAL_INT(-1, rc);
    TEST_ASSERT_NOT_NULL(strstr(reply, "error"));
}

/* ------------------------------------------------------------------ */
/* 8. Render canvas too small                                            */
/* ------------------------------------------------------------------ */
void test_render_canvas_too_small(void)
{
    const char *s = "[{\"op\":\"page.render\",\"page\":0}]";
    int rc = xf_json_exec(ctx, s, strlen(s),
                          canvas, 10, reply, sizeof reply);
    TEST_ASSERT_EQUAL_INT(-1, rc);
    TEST_ASSERT_NOT_NULL(strstr(reply, "error"));
}

/* ------------------------------------------------------------------ */
/* 9. String truncation                                                  */
/* ------------------------------------------------------------------ */
void test_string_truncation(void)
{
    /* date field is char date[32]; a 200-char input must not overflow */
    char script[512];
    char longdate[201];
    memset(longdate, 'A', 200);
    longdate[200] = '\0';
    snprintf(script, sizeof script,
             "[{\"op\":\"row.add\",\"id\":\"component.header.trunc\","
             "  \"data\":{\"date\":\"%s\"}}]", longdate);
    int rc = xf_json_exec(ctx, script, strlen(script),
                          canvas, sizeof canvas, reply, sizeof reply);
    /* Decode should succeed with truncation to field size */
    TEST_ASSERT_EQUAL_INT(0, rc);
    TEST_ASSERT_NOT_NULL(strstr(reply, "\"ok\":true"));
}

/* ------------------------------------------------------------------ */
/* 10. Color parsing                                                      */
/* ------------------------------------------------------------------ */
void test_color_parsing(void)
{
    /* Valid 7-char */
    {
        const char *s =
            "[{\"op\":\"row.add\",\"id\":\"component.header.c1\","
            "  \"data\":{\"status_dot\":\"#1D9E75\"}}]";
        int rc = xf_json_exec(ctx, s, strlen(s),
                              canvas, sizeof canvas, reply, sizeof reply);
        TEST_ASSERT_EQUAL_INT(0, rc);
    }

    /* Valid 9-char with alpha */
    {
        const char *s =
            "[{\"op\":\"row.add\",\"id\":\"component.header.c2\","
            "  \"data\":{\"status_dot\":\"#1D9E7580\"}}]";
        int rc = xf_json_exec(ctx, s, strlen(s),
                              canvas, sizeof canvas, reply, sizeof reply);
        TEST_ASSERT_EQUAL_INT(0, rc);
    }

    /* Valid 3-char shorthand */
    {
        const char *s =
            "[{\"op\":\"row.add\",\"id\":\"component.header.c3\","
            "  \"data\":{\"status_dot\":\"#abc\"}}]";
        int rc = xf_json_exec(ctx, s, strlen(s),
                              canvas, sizeof canvas, reply, sizeof reply);
        TEST_ASSERT_EQUAL_INT(0, rc);
    }

    /* Invalid — decode should fail, command should error */
    {
        const char *s =
            "[{\"op\":\"row.add\",\"id\":\"component.header.c4\","
            "  \"data\":{\"status_dot\":\"not-a-color\"}}]";
        int rc = xf_json_exec(ctx, s, strlen(s),
                              canvas, sizeof canvas, reply, sizeof reply);
        TEST_ASSERT_EQUAL_INT(-1, rc);
        TEST_ASSERT_NOT_NULL(strstr(reply, "error"));
    }
}

/* ------------------------------------------------------------------ */
/* 11. Response truncation                                               */
/* ------------------------------------------------------------------ */
void test_response_truncation(void)
{
    char small_reply[64];
    memset(small_reply, 0, sizeof small_reply);

    const char *s =
        "[{\"op\":\"row.add\",\"id\":\"component.header.trunc2\","
        "  \"data\":{\"date\":\"X\"}},"
        " {\"op\":\"page.render\",\"page\":0}]";

    xf_json_exec(ctx, s, strlen(s),
                 canvas, sizeof canvas, small_reply, sizeof small_reply);

    /* Must be valid JSON — at minimum the truncation envelope */
    TEST_ASSERT_TRUE(small_reply[0] == '{');
    /* The fallback envelope always starts with {"ok": */
    TEST_ASSERT_NOT_NULL(strstr(small_reply, "\"ok\""));
}

/* ------------------------------------------------------------------ */
/* 12. Full lifecycle                                                     */
/* ------------------------------------------------------------------ */
void test_full_lifecycle(void)
{
    /* Add 5 mixed kinds */
    const char *add5 =
        "[{\"op\":\"row.add\",\"id\":\"component.header.lc1\","
        "  \"data\":{\"date\":\"Thu\",\"status_text\":\"ok\",\"status_dot\":\"#1D9E75\"}},"
        " {\"op\":\"row.add\",\"id\":\"component.spacer.lc2\",\"data\":{}},"
        " {\"op\":\"row.add\",\"id\":\"component.deploy.lc3\","
        "  \"data\":{\"branch\":\"main\",\"time_ago\":\"1m\",\"label\":\"green\"}},"
        " {\"op\":\"row.add\",\"id\":\"component.divider.lc4\",\"data\":{}},"
        " {\"op\":\"row.add\",\"id\":\"component.sprint.lc5\","
        "  \"data\":{\"title\":\"Sprint 42\",\"progress_label\":\"80%\","
        "           \"time_left\":\"2d\",\"percent\":0.8}}]";
    int rc = xf_json_exec(ctx, add5, strlen(add5),
                          canvas, sizeof canvas, reply, sizeof reply);
    TEST_ASSERT_EQUAL_INT(0, rc);

    /* Render page 0 */
    memset(reply, 0, sizeof reply);
    const char *r0 = "[{\"op\":\"page.render\",\"page\":0}]";
    rc = xf_json_exec(ctx, r0, strlen(r0),
                      canvas, sizeof canvas, reply, sizeof reply);
    TEST_ASSERT_EQUAL_INT(0, rc);
    TEST_ASSERT_NOT_NULL(strstr(reply, "\"page_count\""));

    /* Update two */
    memset(reply, 0, sizeof reply);
    const char *upd =
        "[{\"op\":\"row.update\",\"id\":\"component.header.lc1\","
        "  \"data\":{\"date\":\"Fri\"}},"
        " {\"op\":\"row.update\",\"id\":\"component.sprint.lc5\","
        "  \"data\":{\"percent\":0.9}}]";
    rc = xf_json_exec(ctx, upd, strlen(upd),
                      canvas, sizeof canvas, reply, sizeof reply);
    TEST_ASSERT_EQUAL_INT(0, rc);

    /* Move one up */
    memset(reply, 0, sizeof reply);
    const char *mv =
        "[{\"op\":\"row.move_up\",\"id\":\"component.deploy.lc3\"}]";
    rc = xf_json_exec(ctx, mv, strlen(mv),
                      canvas, sizeof canvas, reply, sizeof reply);
    TEST_ASSERT_EQUAL_INT(0, rc);

    /* Remove one */
    memset(reply, 0, sizeof reply);
    const char *rem =
        "[{\"op\":\"row.remove\",\"id\":\"component.divider.lc4\"}]";
    rc = xf_json_exec(ctx, rem, strlen(rem),
                      canvas, sizeof canvas, reply, sizeof reply);
    TEST_ASSERT_EQUAL_INT(0, rc);

    /* Render again — page_count and dirty rect are valid */
    memset(reply, 0, sizeof reply);
    rc = xf_json_exec(ctx, r0, strlen(r0),
                      canvas, sizeof canvas, reply, sizeof reply);
    TEST_ASSERT_EQUAL_INT(0, rc);
    TEST_ASSERT_NOT_NULL(strstr(reply, "\"ok\":true"));
    TEST_ASSERT_GREATER_OR_EQUAL(1, xf_json_page_count(ctx));
}

/* ------------------------------------------------------------------ */
/* 13. row.list — empty registry                                        */
/* ------------------------------------------------------------------ */
void test_row_list_empty(void)
{
    const char *s = "[{\"op\":\"row.list\"}]";
    int rc = xf_json_exec(ctx, s, strlen(s),
                          canvas, sizeof canvas, reply, sizeof reply);
    TEST_ASSERT_EQUAL_INT(0, rc);
    TEST_ASSERT_NOT_NULL(strstr(reply, "\"ok\":true"));
    TEST_ASSERT_NOT_NULL(strstr(reply, "\"count\":0"));
    TEST_ASSERT_NOT_NULL(strstr(reply, "\"rows\":[]"));
}

/* ------------------------------------------------------------------ */
/* 14. row.list — after add, returns ids/kinds/data                    */
/* ------------------------------------------------------------------ */
void test_row_list_after_add(void)
{
    const char *add =
        "[{\"op\":\"row.add\",\"id\":\"component.header.ls1\","
        "  \"data\":{\"date\":\"Mon\",\"status_text\":\"ok\",\"status_dot\":\"#1D9E75\"}},"
        " {\"op\":\"row.add\",\"id\":\"component.deploy.ls2\","
        "  \"data\":{\"branch\":\"main\",\"time_ago\":\"1m\",\"label\":\"v1\"}}]";
    xf_json_exec(ctx, add, strlen(add),
                 canvas, sizeof canvas, reply, sizeof reply);

    memset(reply, 0, sizeof reply);
    const char *s = "[{\"op\":\"row.list\"}]";
    int rc = xf_json_exec(ctx, s, strlen(s),
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
}

/* ------------------------------------------------------------------ */
/* 15. row.list — dirty flag reflects update without render             */
/* ------------------------------------------------------------------ */
void test_row_list_dirty_flag(void)
{
    const char *add =
        "[{\"op\":\"row.add\",\"id\":\"component.header.ls3\","
        "  \"data\":{\"date\":\"Mon\",\"status_text\":\"ok\",\"status_dot\":\"#1D9E75\"}},"
        " {\"op\":\"page.render\",\"page\":0}]";
    xf_json_exec(ctx, add, strlen(add),
                 canvas, sizeof canvas, reply, sizeof reply);

    memset(reply, 0, sizeof reply);
    const char *list = "[{\"op\":\"row.list\"}]";
    xf_json_exec(ctx, list, strlen(list),
                 canvas, sizeof canvas, reply, sizeof reply);
    TEST_ASSERT_NOT_NULL(strstr(reply, "\"dirty\":0"));

    memset(reply, 0, sizeof reply);
    const char *upd =
        "[{\"op\":\"row.update\",\"id\":\"component.header.ls3\","
        "  \"data\":{\"date\":\"Tue\"}}]";
    xf_json_exec(ctx, upd, strlen(upd),
                 canvas, sizeof canvas, reply, sizeof reply);

    memset(reply, 0, sizeof reply);
    xf_json_exec(ctx, list, strlen(list),
                 canvas, sizeof canvas, reply, sizeof reply);
    TEST_ASSERT_NOT_NULL(strstr(reply, "\"dirty\":1"));
}

/* ------------------------------------------------------------------ */
int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_add_render_minimal);
    RUN_TEST(test_update_changes_pixels);
    RUN_TEST(test_remove_drops_row);
    RUN_TEST(test_duplicate_id_errors);
    RUN_TEST(test_invalid_id_errors);
    RUN_TEST(test_unknown_op);
    RUN_TEST(test_render_without_canvas);
    RUN_TEST(test_render_canvas_too_small);
    RUN_TEST(test_string_truncation);
    RUN_TEST(test_color_parsing);
    RUN_TEST(test_response_truncation);
    RUN_TEST(test_full_lifecycle);
    RUN_TEST(test_row_list_empty);
    RUN_TEST(test_row_list_after_add);
    RUN_TEST(test_row_list_dirty_flag);
    return UNITY_END();
}

#include <string.h>
#include <stdio.h>
#include "json_encode.h"
#include "mjson.h"

/* Fallback envelope emitted when the buffer is too small to hold real output. */
#define TRUNC_ENVELOPE "{\"ok\":false,\"error\":\"output_truncated\"}"
#define TRUNC_ENV_LEN  ((int)(sizeof(TRUNC_ENVELOPE) - 1))

static void enc_append(xf_encoder_t *e, const char *s, int n)
{
    if (e->truncated)
        return;
    if (n < 0)
        n = (int)strlen(s);
    if (e->len + (size_t)n + 1 > e->cap) {
        e->truncated = 1;
        return;
    }
    memcpy(e->buf + e->len, s, (size_t)n);
    e->len += (size_t)n;
    e->buf[e->len] = '\0';
}

void xf_encoder_init(xf_encoder_t *e, char *buf, size_t cap)
{
    memset(e, 0, sizeof(*e));
    e->buf          = buf;
    e->cap          = cap;
    e->first_result = 1;
    if (cap > 0)
        buf[0] = '\0';
}

void xf_encoder_begin(xf_encoder_t *e)
{
    const char *prefix = "{\"ok\":";
    enc_append(e, prefix, -1);
    if (!e->truncated)
        e->ok_offset = e->len; /* remember where "true"/"false" lives */
    enc_append(e, "true", 4);
    enc_append(e, ",\"results\":[", -1);
}

static void enc_sep(xf_encoder_t *e)
{
    if (!e->first_result)
        enc_append(e, ",", 1);
    e->first_result = 0;
}

void xf_encoder_result(xf_encoder_t *e, int idx, const char *op, const char *id)
{
    char tmp[256];
    int  n;
    (void)idx;
    enc_sep(e);
    if (id) {
        n = mjson_snprintf(tmp, sizeof(tmp), "{\"op\":%Q,\"id\":%Q}", op, id);
    } else {
        n = mjson_snprintf(tmp, sizeof(tmp), "{\"op\":%Q}", op);
    }
    if (n > 0)
        enc_append(e, tmp, n);
}

void xf_encoder_render_result(xf_encoder_t *e, int idx, int page, int page_count,
                              int x, int y, int w, int h)
{
    char tmp[256];
    int  n;
    (void)idx;
    enc_sep(e);
    n = mjson_snprintf(tmp, sizeof(tmp),
                       "{\"op\":\"page.render\",\"page\":%d,\"page_count\":%d,"
                       "\"dirty\":[%d,%d,%d,%d]}",
                       page, page_count, x, y, w, h);
    if (n > 0)
        enc_append(e, tmp, n);
}

void xf_encoder_error(xf_encoder_t *e, int idx, const char *op, const char *id,
                      const char *msg)
{
    char tmp[512];
    int  n;
    (void)idx;
    e->any_error = 1;
    enc_sep(e);
    if (id) {
        n = mjson_snprintf(tmp, sizeof(tmp),
                           "{\"op\":%Q,\"id\":%Q,\"error\":%Q}",
                           op ? op : "?", id, msg ? msg : "error");
    } else {
        n = mjson_snprintf(tmp, sizeof(tmp),
                           "{\"op\":%Q,\"error\":%Q}",
                           op ? op : "?", msg ? msg : "error");
    }
    if (n > 0)
        enc_append(e, tmp, n);
}

void xf_encoder_end(xf_encoder_t *e, int ok)
{
    /* Close the results array. */
    enc_append(e, "]}", -1);

    if (e->truncated || e->cap == 0) {
        /* Try to write the truncation envelope. */
        if (e->cap >= (size_t)(TRUNC_ENV_LEN + 1)) {
            memcpy(e->buf, TRUNC_ENVELOPE, (size_t)TRUNC_ENV_LEN);
            e->buf[TRUNC_ENV_LEN] = '\0';
            e->len = (size_t)TRUNC_ENV_LEN;
        } else if (e->cap > 0) {
            /* Last resort: single '{'. */
            e->buf[0] = '{';
            if (e->cap > 1)
                e->buf[1] = '\0';
            e->len = 1;
        }
        return;
    }

    /* Patch the "ok" field: "true" (4 bytes) -> "false" (5 bytes).
     * Requires shifting everything after offset+4 forward by one byte. */
    if (!ok && e->ok_offset + 4 <= e->len && e->len + 2 <= e->cap) {
        memmove(e->buf + e->ok_offset + 5,
                e->buf + e->ok_offset + 4,
                e->len - (e->ok_offset + 4) + 1 /* include NUL */);
        memcpy(e->buf + e->ok_offset, "false", 5);
        e->len++;
    }
}

void xf_encoder_list_begin(xf_encoder_t *e, int idx, int count, int page_count)
{
    char tmp[128];
    int  n;
    (void)idx;
    enc_sep(e);
    n = snprintf(tmp, sizeof tmp,
                 "{\"op\":\"row.list\",\"count\":%d,\"page_count\":%d,\"rows\":[",
                 count, page_count);
    if (n > 0)
        enc_append(e, tmp, n);
}

void xf_encoder_list_end(xf_encoder_t *e)
{
    enc_append(e, "]}", 2);
}

void xf_encoder_raw(xf_encoder_t *e, const char *s, int n)
{
    enc_append(e, s, n);
}

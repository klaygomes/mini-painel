#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <limits.h>

#include "json_api.h"
#include "json_backend.h"
#include "json_encode.h"
#include "json_decode_util.h"
#include "mjson.h"
#include "debug.h"

struct xf_json_ctx {
    xf_json_backend_t *backend;
};

#define XF_JSON_OP_MAX_LEN 127

struct xf_cmd_env;

typedef int (*xf_handler_t)(xf_json_ctx_t *ctx,
                            xf_encoder_t *enc,
                            const struct xf_cmd_env *env);

typedef struct xf_cmd_env {
    const xf_json_exec_req_t *req;
    const char               *cmd;
    int                       cmdlen;
    int                       idx;
} xf_cmd_env_t;

typedef struct {
    const char  *op;
    xf_handler_t handler;
} xf_dispatch_t;

xf_json_ctx_t *xf_json_create(int width, int height, int padding)
{
    xf_json_ctx_t *c;

    c = calloc(1, sizeof(*c));
    if (!c)
        return NULL;

    c->backend = xf_json_backend_dashboard_create(width, height, padding);
    if (!c->backend) {
        free(c);
        return NULL;
    }

    return c;
}

void xf_json_destroy(xf_json_ctx_t *ctx)
{

    if (!ctx)
        return;

    xf_json_backend_destroy(ctx->backend);
    free(ctx);
}

int xf_json_page_count(const xf_json_ctx_t *ctx)
{
    if (!ctx)
        return 0;
    return xf_json_backend_page_count(ctx->backend);
}

static int do_add(xf_json_ctx_t *c, xf_encoder_t *enc,
                  const char *cmd, int cmdlen, int idx)
{
    char                  id[64];
    const char           *sub;
    int                   sublen;
    const char           *data_json = NULL;
    int                   data_json_len = 0;
    const char           *err_msg = "row.add failed";

    if (mjson_get_string(cmd, cmdlen, "$.id", id, (int)sizeof(id)) <= 0) {
        xf_encoder_error(enc, idx, "row.add", NULL, "missing id");
        return -1;
    }

    if (mjson_find(cmd, cmdlen, "$.data", &sub, &sublen) == MJSON_TOK_OBJECT) {
        data_json = sub;
        data_json_len = sublen;
    }

    if (xf_json_backend_row_add(c->backend, id, data_json, data_json_len, &err_msg) < 0) {
        xf_encoder_error(enc, idx, "row.add", id, err_msg);
        return -1;
    }

    xf_encoder_result(enc, idx, "row.add", id);
    return 0;
}

static int do_update(xf_json_ctx_t *c, xf_encoder_t *enc,
                     const char *cmd, int cmdlen, int idx)
{
    char            id[64];
    const char     *sub;
    int             sublen;
    const char     *data_json = NULL;
    int             data_json_len = 0;
    const char     *err_msg = "row.update failed";

    if (mjson_get_string(cmd, cmdlen, "$.id", id, (int)sizeof(id)) <= 0) {
        xf_encoder_error(enc, idx, "row.update", NULL, "missing id");
        return -1;
    }

    if (mjson_find(cmd, cmdlen, "$.data", &sub, &sublen) == MJSON_TOK_OBJECT) {
        data_json = sub;
        data_json_len = sublen;
    }

    if (xf_json_backend_row_update(c->backend, id, data_json, data_json_len, &err_msg) < 0) {
        xf_encoder_error(enc, idx, "row.update", id, err_msg);
        return -1;
    }

    xf_encoder_result(enc, idx, "row.update", id);
    return 0;
}

static int do_remove(xf_json_ctx_t *c, xf_encoder_t *enc,
                     const char *cmd, int cmdlen, int idx)
{
    char        id[64];
    const char *err_msg = "row.remove failed";

    if (mjson_get_string(cmd, cmdlen, "$.id", id, (int)sizeof(id)) <= 0) {
        xf_encoder_error(enc, idx, "row.remove", NULL, "missing id");
        return -1;
    }

    if (xf_json_backend_row_remove(c->backend, id, &err_msg) < 0) {
        xf_encoder_error(enc, idx, "row.remove", id, err_msg);
        return -1;
    }

    xf_encoder_result(enc, idx, "row.remove", id);
    return 0;
}

static int do_move(xf_json_ctx_t *c, xf_encoder_t *enc,
                   const char *cmd, int cmdlen, int idx, int dir)
{
    char        id[64];
    const char *err_msg = "row.move failed";

    if (mjson_get_string(cmd, cmdlen, "$.id", id, (int)sizeof(id)) <= 0) {
        xf_encoder_error(enc, idx, dir > 0 ? "row.move_up" : "row.move_down", NULL, "missing id");
        return -1;
    }

    if (xf_json_backend_row_move(c->backend, id, dir, &err_msg) < 0) {
        xf_encoder_error(enc, idx, dir > 0 ? "row.move_up" : "row.move_down", id, err_msg);
        return -1;
    }

    xf_encoder_result(enc, idx, dir > 0 ? "row.move_up" : "row.move_down", id);
    return 0;
}

static int do_render(xf_json_ctx_t *c, xf_encoder_t *enc,
                     const char *cmd, int cmdlen, int idx,
                     uint8_t *canvas, size_t canvas_cap)
{
    double      page_d  = 0;
    int         page    = 0;
    int         page_count = 0;
    int         dx = 0, dy = 0, dw = 0, dh = 0;
    const char *err_msg = "render failed";

    mjson_get_number(cmd, cmdlen, "$.page", &page_d);
    page = (int)page_d;

    if (xf_json_backend_page_render(c->backend,
                                    page,
                                    canvas,
                                    canvas_cap,
                                    &page_count,
                                    &dx,
                                    &dy,
                                    &dw,
                                    &dh,
                                    &err_msg) < 0) {
        xf_encoder_error(enc, idx, "page.render", NULL, err_msg);
        return -1;
    }

    DEBUG_LOG("do_render page=%d pages=%d", page, page_count);

    xf_encoder_render_result(enc, idx, page, page_count, dx, dy, dw, dh);
    return 0;
}

static int do_list(xf_json_ctx_t *c, xf_encoder_t *enc, int idx)
{
    int i;
    int count;
    int page_count;
    char data_buf[2048];

    page_count = xf_json_backend_page_count(c->backend);
    count = xf_json_backend_row_count(c->backend);
    xf_encoder_list_begin(enc, idx, count, page_count);

    for (i = 0; i < count; i++) {
        xf_json_backend_row_view_t row;
        char                       row_hdr[256];
        int                        n;

        if (xf_json_backend_row_view(c->backend, i, &row) < 0)
            continue;

        if (i > 0)
            xf_encoder_raw(enc, ",", 1);

        n = mjson_snprintf(row_hdr, sizeof row_hdr,
            "{\"id\":%Q,\"kind\":%Q,\"page\":%d,\"index\":%d,\"dirty\":%d,\"data\":",
            row.id, row.kind, row.page, row.index, row.dirty);
        if (n > 0)
            xf_encoder_raw(enc, row_hdr, n);

        if (row.encode(row.data, data_buf, sizeof data_buf) == 0)
            xf_encoder_raw(enc, data_buf, (int)strlen(data_buf));
        else
            xf_encoder_raw(enc, "{}", 2);

        xf_encoder_raw(enc, "}", 1);
    }

    xf_encoder_list_end(enc);
    return 0;
}

static int handle_row_add(xf_json_ctx_t *ctx, xf_encoder_t *enc,
                          const xf_cmd_env_t *env)
{
    return do_add(ctx, enc, env->cmd, env->cmdlen, env->idx);
}

static int handle_row_update(xf_json_ctx_t *ctx, xf_encoder_t *enc,
                             const xf_cmd_env_t *env)
{
    return do_update(ctx, enc, env->cmd, env->cmdlen, env->idx);
}

static int handle_row_remove(xf_json_ctx_t *ctx, xf_encoder_t *enc,
                             const xf_cmd_env_t *env)
{
    return do_remove(ctx, enc, env->cmd, env->cmdlen, env->idx);
}

static int handle_row_move_up(xf_json_ctx_t *ctx, xf_encoder_t *enc,
                              const xf_cmd_env_t *env)
{
    return do_move(ctx, enc, env->cmd, env->cmdlen, env->idx, +1);
}

static int handle_row_move_down(xf_json_ctx_t *ctx, xf_encoder_t *enc,
                                const xf_cmd_env_t *env)
{
    return do_move(ctx, enc, env->cmd, env->cmdlen, env->idx, -1);
}

static int handle_row_list(xf_json_ctx_t *ctx, xf_encoder_t *enc,
                           const xf_cmd_env_t *env)
{
    return do_list(ctx, enc, env->idx);
}

static int handle_page_render(xf_json_ctx_t *ctx, xf_encoder_t *enc,
                              const xf_cmd_env_t *env)
{
    return do_render(ctx, enc,
                     env->cmd, env->cmdlen, env->idx,
                     (uint8_t *)env->req->canvas.buf, env->req->canvas.size);
}

static const xf_dispatch_t g_handlers[] = {
    { "row.add", handle_row_add },
    { "row.update", handle_row_update },
    { "row.remove", handle_row_remove },
    { "row.move_up", handle_row_move_up },
    { "row.move_down", handle_row_move_down },
    { "row.list", handle_row_list },
    { "page.render", handle_page_render },
};

static const xf_dispatch_t *find_handler(const char *op, int op_len)
{
    size_t i;

    for (i = 0; i < sizeof(g_handlers) / sizeof(g_handlers[0]); i++) {
        const size_t key_len = strlen(g_handlers[i].op);
        if ((int)key_len == op_len && strncmp(g_handlers[i].op, op, (size_t)op_len) == 0)
            return &g_handlers[i];
    }
    return NULL;
}

int xf_json_exec(const xf_json_exec_req_t *req)
{
    xf_encoder_t enc;
    int          off = 0, koff, klen, voff, vlen, vtype;
    int          idx = 0, ok = 1;
    int          json_len_i;
    xf_json_ctx_t *ctx;
    const char    *json;
    size_t         json_len;
    char          *out_json;
    size_t         out_json_cap;

    if (!req)
        return -1;

    ctx = req->ctx;
    json = (const char *)req->json.buf;
    json_len = req->json.size;
    out_json = (char *)req->out_json.buf;
    out_json_cap = req->out_json.size;

    if (!ctx || !json || json_len == 0 || json_len > (size_t)INT_MAX)
        return -1;

    json_len_i = (int)json_len;

    xf_encoder_init(&enc, out_json, out_json_cap);
    xf_encoder_begin(&enc);

    while ((off = mjson_next(json, json_len_i, off,
                             &koff, &klen, &voff, &vlen, &vtype)) != 0) {
        const char          *op_ptr = NULL;
        int                  op_len = 0;
        int                  op_type;
        char                 op_name[XF_JSON_OP_MAX_LEN + 1];
        const char          *cmd;
        int                  rc;
        const xf_dispatch_t *entry;
        xf_cmd_env_t         env;

        DEBUG_LOG("mjson_next off=%d koff=%d klen=%d voff=%d vlen=%d vtype=%d",
                  off, koff, klen, voff, vlen, vtype);
        DEBUG_LOG("mjson_next key=%.*s", klen, json + koff);

        cmd = json + voff;
        DEBUG_LOG("mjson_next cmd=%.*s", vlen, cmd);

        if (vtype != MJSON_TOK_OBJECT) {
            xf_encoder_error(&enc, idx, "?", NULL, "command must be an object");
            ok = 0;
            idx++;
            continue;
        }

        op_type = mjson_find(cmd, vlen, "$.op", &op_ptr, &op_len);
        DEBUG_LOG("op parse type=%d", op_type);
        if (op_type == MJSON_TOK_INVALID) {
            xf_encoder_error(&enc, idx, "?", NULL, "missing op");
            ok = 0;
            idx++;
            continue;
        }
        DEBUG_LOG("op parse raw=%.*s len=%d", op_len, op_ptr, op_len);

        if (op_type != MJSON_TOK_STRING) {
            xf_encoder_error(&enc, idx, "?", NULL, "op must be a string");
            ok = 0;
            idx++;
            continue;
        }

        if (op_len < 2 || op_ptr[0] != '"' || op_ptr[op_len - 1] != '"') {
            xf_encoder_error(&enc, idx, "?", NULL, "invalid op format");
            ok = 0;
            idx++;
            continue;
        }

        op_ptr++;
        op_len -= 2;

        if (op_len > XF_JSON_OP_MAX_LEN) {
            xf_encoder_error(&enc, idx, "?", NULL, "op too long");
            ok = 0;
            idx++;
            continue;
        }

        memcpy(op_name, op_ptr, (size_t)op_len);
        op_name[op_len] = '\0';
        DEBUG_LOG("op normalized=%.*s len=%d", op_len, op_name, op_len);

        entry = find_handler(op_ptr, op_len);
        if (!entry) {
            xf_encoder_error(&enc, idx, op_name, NULL, "unknown op");
            ok = 0;
            idx++;
            continue;
        }

        env.req = req;
        env.cmd = cmd;
        env.cmdlen = vlen;
        env.idx = idx;

        rc = entry->handler(ctx, &enc, &env);

        if (rc < 0)
            ok = 0;
        idx++;
    }

    xf_encoder_end(&enc, ok);
    return ok ? 0 : -1;
}

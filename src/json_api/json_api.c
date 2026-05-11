#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <limits.h>

#include "json_api.h"
#include "json_backend.h"
#include "json_str_slice.h"
#include "json_encode.h"
#include "json_decode_util.h"
#include "mjson.h"
#include "debug.h"

struct xf_json_ctx {
    xf_json_backend_t *backend;
};

#define XF_JSON_OP_MAX_LEN 127

typedef struct xf_cmd_env {
    const xf_json_exec_req_t *req;
    const char               *cmd;
    int                       cmdlen;
    int                       idx;
} xf_cmd_env_t;

typedef struct xf_handler_ctx {
    xf_json_ctx_t      *ctx;
    xf_encoder_t       *enc;
    const xf_cmd_env_t *env;
} xf_handler_ctx_t;

typedef int (*xf_handler_t)(const xf_handler_ctx_t *hctx, const void *arg);

typedef struct {
    const char  *op;
    xf_handler_t handler;
    const void  *arg;
} xf_dispatch_t;

typedef struct {
    const char *op;
    int (*backend_fn)(xf_json_backend_t *backend,
                      const char *id,
                      xf_str_slice_t data_json,
                      const char **err_msg);
} xf_row_data_op_t;

typedef struct {
    const char *op;
    int         dir;
} xf_move_op_t;

static int xf_cmd_get_id(const xf_cmd_env_t *env, char *id, size_t id_size)
{
    return mjson_get_string(env->cmd, env->cmdlen, "$.id", id, (int)id_size) > 0 ? 0 : -1;
}

static xf_str_slice_t xf_cmd_get_data_object(const xf_cmd_env_t *env)
{
    const char     *sub;
    int             sublen;
    xf_str_slice_t  result = {NULL, 0};

    if (mjson_find(env->cmd, env->cmdlen, "$.data", &sub, &sublen) == MJSON_TOK_OBJECT) {
        result.ptr = sub;
        result.len = sublen;
    }
    return result;
}

static int xf_handler_missing_id(const xf_handler_ctx_t *hctx, const char *op)
{
    xf_encoder_error(hctx->enc, hctx->env->idx, op, NULL, "missing id");
    return -1;
}

static int xf_handler_result(const xf_handler_ctx_t *hctx, const char *op, const char *id)
{
    xf_encoder_result(hctx->enc, hctx->env->idx, op, id);
    return 0;
}

static int xf_handler_error(const xf_handler_ctx_t *hctx,
                            const char *op,
                            const char *id,
                            const char *err_msg)
{
    xf_encoder_error(hctx->enc, hctx->env->idx, op, id, err_msg);
    return -1;
}

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

static int handle_row_data_op(const xf_handler_ctx_t *hctx, const void *arg)
{
    const xf_row_data_op_t *op = (const xf_row_data_op_t *)arg;
    char            id[64];
    xf_str_slice_t  data_json;
    const char     *err_msg;

    if (xf_cmd_get_id(hctx->env, id, sizeof(id)) < 0)
        return xf_handler_missing_id(hctx, op->op);

    data_json = xf_cmd_get_data_object(hctx->env);

    err_msg = op->op;
    if (op->backend_fn(hctx->ctx->backend, id, data_json, &err_msg) < 0)
        return xf_handler_error(hctx, op->op, id, err_msg);

    return xf_handler_result(hctx, op->op, id);
}

static int handle_row_remove(const xf_handler_ctx_t *hctx, const void *arg)
{
    const char *op = (const char *)arg;
    char        id[64];
    const char *err_msg = "row.remove failed";

    if (xf_cmd_get_id(hctx->env, id, sizeof(id)) < 0)
        return xf_handler_missing_id(hctx, op);

    if (xf_json_backend_row_remove(hctx->ctx->backend, id, &err_msg) < 0)
        return xf_handler_error(hctx, op, id, err_msg);

    return xf_handler_result(hctx, op, id);
}

static int handle_row_move(const xf_handler_ctx_t *hctx, const void *arg)
{
    const xf_move_op_t *op = (const xf_move_op_t *)arg;
    char                id[64];
    const char         *err_msg = "row.move failed";

    if (xf_cmd_get_id(hctx->env, id, sizeof(id)) < 0)
        return xf_handler_missing_id(hctx, op->op);

    if (xf_json_backend_row_move(hctx->ctx->backend, id, op->dir, &err_msg) < 0)
        return xf_handler_error(hctx, op->op, id, err_msg);

    return xf_handler_result(hctx, op->op, id);
}

static int handle_row_list(const xf_handler_ctx_t *hctx, const void *arg)
{
    int  i;
    int  count;
    int  page_count;
    char data_buf[2048];

    (void)arg;

    page_count = xf_json_backend_page_count(hctx->ctx->backend);
    count = xf_json_backend_row_count(hctx->ctx->backend);
    xf_encoder_list_begin(hctx->enc, hctx->env->idx, count, page_count);

    for (i = 0; i < count; i++) {
        xf_json_backend_row_view_t row;
        char                       row_hdr[256];
        int                        n;

        if (xf_json_backend_row_view(hctx->ctx->backend, i, &row) < 0)
            continue;

        if (i > 0)
            xf_encoder_raw(hctx->enc, (xf_str_slice_t){",", 1});

        n = mjson_snprintf(row_hdr, sizeof row_hdr,
            "{\"id\":%Q,\"kind\":%Q,\"page\":%d,\"index\":%d,\"dirty\":%d,\"data\":",
            row.id, row.kind, row.page, row.index, row.dirty);
        if (n > 0)
            xf_encoder_raw(hctx->enc, (xf_str_slice_t){row_hdr, n});

        if (row.encode(row.data, (xf_out_buf_t){data_buf, sizeof data_buf}) == 0)
            xf_encoder_raw(hctx->enc, (xf_str_slice_t){data_buf, (int)strlen(data_buf)});
        else
            xf_encoder_raw(hctx->enc, (xf_str_slice_t){"{}", 2});

        xf_encoder_raw(hctx->enc, (xf_str_slice_t){"}", 1});
    }

    xf_encoder_list_end(hctx->enc);
    return 0;
}

static int handle_page_render(const xf_handler_ctx_t *hctx, const void *arg)
{
    const char *op = (const char *)arg;
    double      page_d  = 0;
    int         page    = 0;
    int         page_count = 0;
    int         dx = 0, dy = 0, dw = 0, dh = 0;
    const char *err_msg = "render failed";

    mjson_get_number(hctx->env->cmd, hctx->env->cmdlen, "$.page", &page_d);
    page = (int)page_d;

    if (xf_json_backend_page_render(hctx->ctx->backend,
                                    page,
                                    (xf_byte_buf_t){(uint8_t *)hctx->env->req->canvas.buf,
                                                    hctx->env->req->canvas.size},
                                    &page_count,
                                    &dx,
                                    &dy,
                                    &dw,
                                    &dh,
                                    &err_msg) < 0) {
        return xf_handler_error(hctx, op, NULL, err_msg);
    }

    DEBUG_LOG("do_render page=%d pages=%d", page, page_count);

    xf_encoder_render_result(hctx->enc, hctx->env->idx, page, page_count, dx, dy, dw, dh);
    return 0;
}

static const xf_row_data_op_t g_row_add_op = {
    "row.add",
    xf_json_backend_row_add,
};

static const xf_row_data_op_t g_row_update_op = {
    "row.update",
    xf_json_backend_row_update,
};

static const xf_move_op_t g_row_move_up_op = {
    "row.move_up",
    +1,
};

static const xf_move_op_t g_row_move_down_op = {
    "row.move_down",
    -1,
};

static const xf_dispatch_t g_handlers[] = {
    { "row.add", handle_row_data_op, &g_row_add_op },
    { "row.update", handle_row_data_op, &g_row_update_op },
    { "row.remove", handle_row_remove, "row.remove" },
    { "row.move_up", handle_row_move, &g_row_move_up_op },
    { "row.move_down", handle_row_move, &g_row_move_down_op },
    { "row.list", handle_row_list, NULL },
    { "page.render", handle_page_render, "page.render" },
};

static const xf_dispatch_t *find_handler(xf_str_slice_t op)
{
    size_t i;

    for (i = 0; i < sizeof(g_handlers) / sizeof(g_handlers[0]); i++) {
        const size_t key_len = strlen(g_handlers[i].op);
        if ((int)key_len == op.len && strncmp(g_handlers[i].op, op.ptr, (size_t)op.len) == 0)
            return &g_handlers[i];
    }
    return NULL;
}

int xf_json_exec(const xf_json_exec_req_t *req)
{
    xf_encoder_t      enc;
    int               off = 0, koff, klen, voff, vlen, vtype;
    int               idx = 0, ok = 1;
    int               json_len_i;
    xf_json_ctx_t    *ctx;
    const char       *json;
    size_t            json_len;
    char             *out_json;
    size_t            out_json_cap;

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

    xf_encoder_init(&enc, (xf_out_buf_t){out_json, out_json_cap});
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
        xf_handler_ctx_t     hctx;

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

        entry = find_handler((xf_str_slice_t){op_ptr, op_len});
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

        hctx.ctx = ctx;
        hctx.enc = &enc;
        hctx.env = &env;

        rc = entry->handler(&hctx, entry->arg);

        if (rc < 0)
            ok = 0;
        idx++;
    }

    xf_encoder_end(&enc, ok);
    return ok ? 0 : -1;
}

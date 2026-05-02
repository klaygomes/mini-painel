#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "json_api.h"
#include "json_kind.h"
#include "json_registry.h"
#include "json_encode.h"
#include "json_decode_util.h"
#include "dashboard.h"
#include "mjson.h"
#include "debug.h"

struct xf_json_ctx {
    xf_dashboard_t *dash;
    xf_registry_t   reg;
    int             width, height;
};

xf_json_ctx_t *xf_json_create(int width, int height, int padding)
{
    xf_json_ctx_t *c;
    if (width <= 0 || height <= 0 || padding < 0)
        return NULL;
    c = calloc(1, sizeof(*c));
    if (!c)
        return NULL;
    c->dash = dashboard_create(width, height, padding);
    if (!c->dash) {
        free(c);
        return NULL;
    }
    c->width  = width;
    c->height = height;
    return c;
}

void xf_json_destroy(xf_json_ctx_t *ctx)
{
    int i;
    if (!ctx)
        return;
    for (i = 0; i < ctx->reg.count; i++) {
        free(ctx->reg.entries[i].comp);
        free(ctx->reg.entries[i].data);
    }
    dashboard_destroy(ctx->dash);
    free(ctx);
}

int xf_json_page_count(const xf_json_ctx_t *ctx)
{
    if (!ctx)
        return 0;
    return dashboard_page_count(ctx->dash);
}

static int do_add(xf_json_ctx_t *c, xf_encoder_t *enc,
                  const char *cmd, int cmdlen, int idx)
{
    char                  id[64];
    const xf_kind_def_t  *def;
    xf_reg_entry_t       *entry;
    void                 *data = NULL;
    xf_component_t       *comp;
    const char           *sub;
    int                   sublen;

    if (mjson_get_string(cmd, cmdlen, "$.id", id, (int)sizeof(id)) <= 0) {
        xf_encoder_error(enc, idx, "row.add", NULL, "missing id");
        return -1;
    }

    def = xf_registry_kind_from_id(id);
    if (!def) {
        xf_encoder_error(enc, idx, "row.add", id, "invalid id");
        return -1;
    }

    entry = xf_registry_add(&c->reg, id);
    if (!entry) {
        xf_encoder_error(enc, idx, "row.add", id, "duplicate id or registry full");
        return -1;
    }

    if (!def->stateless) {
        data = calloc(1, def->data_size);
        if (!data) {
            xf_registry_remove(&c->reg, id);
            xf_encoder_error(enc, idx, "row.add", id, "out of memory");
            return -1;
        }
        if (mjson_find(cmd, cmdlen, "$.data", &sub, &sublen) == MJSON_TOK_OBJECT) {
            if (def->decode(sub, sublen, data, 0) < 0) {
                free(data);
                xf_registry_remove(&c->reg, id);
                xf_encoder_error(enc, idx, "row.add", id, "decode error");
                return -1;
            }
        }
    }

    comp = malloc(sizeof(*comp));
    if (!comp) {
        free(data);
        xf_registry_remove(&c->reg, id);
        xf_encoder_error(enc, idx, "row.add", id, "out of memory");
        return -1;
    }
    *comp = def->create(data);

    DEBUG_LOG("do_add data=%p comp->ctx=%p", (void *)data, (void *)comp->ctx);
    if (data) {
        DEBUG_LOG("do_add date='%.31s' status_text='%.31s'",
                  (const char *)data, ((const char *)data) + 32);
    }

    if (dashboard_add_full_row(c->dash, comp, def->height) < 0) {
        free(comp);
        free(data);
        xf_registry_remove(&c->reg, id);
        xf_encoder_error(enc, idx, "row.add", id, "dashboard add failed");
        return -1;
    }

    entry->def  = def;
    entry->comp = comp;
    entry->data = data;

    xf_encoder_result(enc, idx, "row.add", id);
    return 0;
}

static int do_update(xf_json_ctx_t *c, xf_encoder_t *enc,
                     const char *cmd, int cmdlen, int idx)
{
    char            id[64];
    xf_reg_entry_t *entry;
    const char     *sub;
    int             sublen;

    if (mjson_get_string(cmd, cmdlen, "$.id", id, (int)sizeof(id)) <= 0) {
        xf_encoder_error(enc, idx, "row.update", NULL, "missing id");
        return -1;
    }

    entry = xf_registry_find(&c->reg, id);
    if (!entry) {
        xf_encoder_error(enc, idx, "row.update", id, "id not found");
        return -1;
    }

    if (!entry->def->stateless && entry->data) {
        if (mjson_find(cmd, cmdlen, "$.data", &sub, &sublen) == MJSON_TOK_OBJECT) {
            if (entry->def->decode(sub, sublen, entry->data, 1) < 0) {
                xf_encoder_error(enc, idx, "row.update", id, "decode error");
                return -1;
            }
        }
    }
    entry->comp->dirty = 1;

    xf_encoder_result(enc, idx, "row.update", id);
    return 0;
}

static int do_remove(xf_json_ctx_t *c, xf_encoder_t *enc,
                     const char *cmd, int cmdlen, int idx)
{
    char            id[64];
    xf_reg_entry_t *entry;
    int             row_idx;
    xf_component_t *comp;
    void           *data;

    if (mjson_get_string(cmd, cmdlen, "$.id", id, (int)sizeof(id)) <= 0) {
        xf_encoder_error(enc, idx, "row.remove", NULL, "missing id");
        return -1;
    }

    entry = xf_registry_find(&c->reg, id);
    if (!entry) {
        xf_encoder_error(enc, idx, "row.remove", id, "id not found");
        return -1;
    }

    row_idx = dashboard_find_row(c->dash, entry->comp);
    if (row_idx < 0) {
        xf_encoder_error(enc, idx, "row.remove", id, "row not found");
        return -1;
    }

    comp = entry->comp;
    data = entry->data;

    dashboard_remove_row(c->dash, row_idx);

    xf_registry_remove(&c->reg, id);
    free(comp);
    free(data);

    xf_encoder_result(enc, idx, "row.remove", id);
    return 0;
}

static int do_move(xf_json_ctx_t *c, xf_encoder_t *enc,
                   const char *cmd, int cmdlen, int idx, int dir)
{
    char            id[64];
    xf_reg_entry_t *entry;
    int             row_idx;
    int             rc;

    if (mjson_get_string(cmd, cmdlen, "$.id", id, (int)sizeof(id)) <= 0) {
        xf_encoder_error(enc, idx, dir > 0 ? "row.move_up" : "row.move_down", NULL, "missing id");
        return -1;
    }

    entry = xf_registry_find(&c->reg, id);
    if (!entry) {
        xf_encoder_error(enc, idx, dir > 0 ? "row.move_up" : "row.move_down", id, "id not found");
        return -1;
    }

    row_idx = dashboard_find_row(c->dash, entry->comp);
    if (row_idx < 0) {
        xf_encoder_error(enc, idx, dir > 0 ? "row.move_up" : "row.move_down", id, "row not found");
        return -1;
    }

    rc = (dir > 0) ? dashboard_move_row_up(c->dash, row_idx)
                   : dashboard_move_row_down(c->dash, row_idx);
    if (rc < 0) {
        xf_encoder_error(enc, idx, dir > 0 ? "row.move_up" : "row.move_down", id, "cannot move");
        return -1;
    }

    xf_encoder_result(enc, idx, dir > 0 ? "row.move_up" : "row.move_down", id);
    return 0;
}

static int do_render(xf_json_ctx_t *c, xf_encoder_t *enc,
                     const char *cmd, int cmdlen, int idx,
                     uint8_t *canvas, size_t canvas_cap)
{
    double        page_d  = 0;
    int           page    = 0;
    int           page_count;
    const uint8_t *frame;
    int            dx = 0, dy = 0, dw = 0, dh = 0;
    size_t         needed = XF_JSON_CANVAS_SIZE(c->width, c->height);
    int            rc;

    if (!canvas || canvas_cap < needed) {
        xf_encoder_error(enc, idx, "page.render", NULL, "canvas too small or NULL");
        return -1;
    }

    mjson_get_number(cmd, cmdlen, "$.page", &page_d);
    page = (int)page_d;

    /* Compute dirty rect before render so it reflects this frame's changes. */
    dashboard_dirty_rect(c->dash, page, &dx, &dy, &dw, &dh);

    rc = dashboard_render_page(c->dash, page, &frame);
    if (rc < 0) {
        xf_encoder_error(enc, idx, "page.render", NULL, "render failed");
        return -1;
    }

    page_count = dashboard_page_count(c->dash);

    memcpy(canvas, frame, needed);

    DEBUG_LOG("do_render page=%d pages=%d", page, page_count);

    xf_encoder_render_result(enc, idx, page, page_count, dx, dy, dw, dh);
    return 0;
}

int xf_json_exec(xf_json_ctx_t *ctx,
                 const char *json, size_t json_len,
                 uint8_t *canvas, size_t canvas_cap,
                 char *out_json, size_t out_json_cap)
{
    xf_encoder_t enc;
    int          off = 0, koff, klen, voff, vlen, vtype;
    int          idx = 0, ok = 1;

    if (!ctx || !json || json_len == 0)
        return -1;

    xf_encoder_init(&enc, out_json, out_json_cap);
    xf_encoder_begin(&enc);

    while ((off = mjson_next(json, (int)json_len, off,
                             &koff, &klen, &voff, &vlen, &vtype)) != 0) {
        char op[64];
        int  rc;

        if (vtype != MJSON_TOK_OBJECT) {
            xf_encoder_error(&enc, idx, "?", NULL, "command must be an object");
            ok = 0;
            break;
        }

        if (mjson_get_string(json + voff, vlen, "$.op", op, (int)sizeof(op)) <= 0) {
            xf_encoder_error(&enc, idx, "?", NULL, "missing op");
            ok = 0;
            break;
        }

        if      (!strcmp(op, "row.add"))       rc = do_add   (ctx, &enc, json+voff, vlen, idx);
        else if (!strcmp(op, "row.update"))    rc = do_update(ctx, &enc, json+voff, vlen, idx);
        else if (!strcmp(op, "row.remove"))    rc = do_remove(ctx, &enc, json+voff, vlen, idx);
        else if (!strcmp(op, "row.move_up"))   rc = do_move  (ctx, &enc, json+voff, vlen, idx, +1);
        else if (!strcmp(op, "row.move_down")) rc = do_move  (ctx, &enc, json+voff, vlen, idx, -1);
        else if (!strcmp(op, "page.render"))   rc = do_render(ctx, &enc, json+voff, vlen, idx, canvas, canvas_cap);
        else {
            xf_encoder_error(&enc, idx, op, NULL, "unknown op");
            rc = -1;
        }

        if (rc < 0) {
            ok = 0;
            break;
        }
        idx++;
    }

    xf_encoder_end(&enc, ok);
    return ok ? 0 : -1;
}

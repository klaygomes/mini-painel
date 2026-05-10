#include <stdlib.h>
#include <string.h>

#include "json_backend.h"
#include "json_registry.h"
#include "dashboard.h"

typedef struct {
    void (*destroy)(xf_json_backend_t *backend);
    int  (*page_count)(const xf_json_backend_t *backend);
    int  (*row_add)(xf_json_backend_t *backend,
                    const char *id,
                    xf_str_slice_t data_json,
                    const char **err_msg);
    int  (*row_update)(xf_json_backend_t *backend,
                       const char *id,
                       xf_str_slice_t data_json,
                       const char **err_msg);
    int  (*row_remove)(xf_json_backend_t *backend,
                       const char *id,
                       const char **err_msg);
    int  (*row_move)(xf_json_backend_t *backend,
                     const char *id,
                     int dir,
                     const char **err_msg);
    int  (*page_render)(xf_json_backend_t *backend,
                        int page,
                        uint8_t *canvas,
                        size_t canvas_cap,
                        int *page_count,
                        int *dx,
                        int *dy,
                        int *dw,
                        int *dh,
                        const char **err_msg);
    int  (*row_count)(const xf_json_backend_t *backend);
    int  (*row_view)(const xf_json_backend_t *backend,
                     int idx,
                     xf_json_backend_row_view_t *view);
} xf_json_backend_ops_t;

struct xf_json_backend {
    const xf_json_backend_ops_t *ops;
    xf_dashboard_t              *dash;
    xf_registry_t                reg;
    int                          width;
    int                          height;
};

static void dashboard_destroy_backend(xf_json_backend_t *backend)
{
    int i;

    if (!backend)
        return;

    for (i = 0; i < backend->reg.count; i++) {
        free(backend->reg.entries[i].comp);
        free(backend->reg.entries[i].data);
    }

    dashboard_destroy(backend->dash);
    free(backend);
}

static int dashboard_backend_page_count(const xf_json_backend_t *backend)
{
    if (!backend)
        return 0;
    return dashboard_page_count(backend->dash);
}

static int dashboard_row_add(xf_json_backend_t *backend,
                             const char *id,
                             xf_str_slice_t data_json,
                             const char **err_msg)
{
    const xf_kind_def_t *def;
    xf_reg_entry_t      *entry;
    void                *data = NULL;
    xf_component_t      *comp;

    if (err_msg)
        *err_msg = "dashboard add failed";

    def = xf_registry_kind_from_id(id);
    if (!def) {
        if (err_msg)
            *err_msg = "invalid id";
        return -1;
    }

    entry = xf_registry_add(&backend->reg, id);
    if (!entry) {
        if (err_msg)
            *err_msg = "duplicate id or registry full";
        return -1;
    }

    if (!def->stateless) {
        data = calloc(1, def->data_size);
        if (!data) {
            xf_registry_remove(&backend->reg, id);
            if (err_msg)
                *err_msg = "out of memory";
            return -1;
        }
        if (data_json.ptr && data_json.len > 0) {
            if (def->decode(data_json, data, 0) < 0) {
                free(data);
                xf_registry_remove(&backend->reg, id);
                if (err_msg)
                    *err_msg = "decode error";
                return -1;
            }
        }
    }

    comp = malloc(sizeof(*comp));
    if (!comp) {
        free(data);
        xf_registry_remove(&backend->reg, id);
        if (err_msg)
            *err_msg = "out of memory";
        return -1;
    }

    *comp = def->create(data);

    if (dashboard_add_full_row(backend->dash, comp, def->height) < 0) {
        free(comp);
        free(data);
        xf_registry_remove(&backend->reg, id);
        if (err_msg)
            *err_msg = "dashboard add failed";
        return -1;
    }

    entry->def  = def;
    entry->comp = comp;
    entry->data = data;
    return 0;
}

static int dashboard_row_update(xf_json_backend_t *backend,
                                const char *id,
                                xf_str_slice_t data_json,
                                const char **err_msg)
{
    xf_reg_entry_t *entry;

    if (err_msg)
        *err_msg = "id not found";

    entry = xf_registry_find(&backend->reg, id);
    if (!entry)
        return -1;

    if (!entry->def->stateless && entry->data && data_json.ptr && data_json.len > 0) {
        if (entry->def->decode(data_json, entry->data, 1) < 0) {
            if (err_msg)
                *err_msg = "decode error";
            return -1;
        }
    }

    entry->comp->dirty = 1;
    return 0;
}

static int dashboard_row_remove(xf_json_backend_t *backend,
                                const char *id,
                                const char **err_msg)
{
    xf_reg_entry_t *entry;
    int             row_idx;
    xf_component_t *comp;
    void           *data;

    if (err_msg)
        *err_msg = "id not found";

    entry = xf_registry_find(&backend->reg, id);
    if (!entry)
        return -1;

    row_idx = dashboard_find_row(backend->dash, entry->comp);
    if (row_idx < 0) {
        if (err_msg)
            *err_msg = "row not found";
        return -1;
    }

    comp = entry->comp;
    data = entry->data;

    dashboard_remove_row(backend->dash, row_idx);
    xf_registry_remove(&backend->reg, id);
    free(comp);
    free(data);
    return 0;
}

static int dashboard_row_move(xf_json_backend_t *backend,
                              const char *id,
                              int dir,
                              const char **err_msg)
{
    xf_reg_entry_t *entry;
    int             row_idx;
    int             rc;

    if (err_msg)
        *err_msg = "id not found";

    entry = xf_registry_find(&backend->reg, id);
    if (!entry)
        return -1;

    row_idx = dashboard_find_row(backend->dash, entry->comp);
    if (row_idx < 0) {
        if (err_msg)
            *err_msg = "row not found";
        return -1;
    }

    rc = (dir > 0) ? dashboard_move_row_up(backend->dash, row_idx)
                   : dashboard_move_row_down(backend->dash, row_idx);
    if (rc < 0) {
        if (err_msg)
            *err_msg = "cannot move";
        return -1;
    }

    return 0;
}

static int dashboard_page_render_op(xf_json_backend_t *backend,
                                    int page,
                                    uint8_t *canvas,
                                    size_t canvas_cap,
                                    int *page_count,
                                    int *dx,
                                    int *dy,
                                    int *dw,
                                    int *dh,
                                    const char **err_msg)
{
    const uint8_t *frame;
    size_t         needed;
    int            rc;

    if (err_msg)
        *err_msg = "render failed";

    needed = (size_t)backend->width * (size_t)backend->height * 3u;
    if (!canvas || canvas_cap < needed) {
        if (err_msg)
            *err_msg = "canvas too small or NULL";
        return -1;
    }

    dashboard_dirty_rect(backend->dash, page, dx, dy, dw, dh);

    rc = dashboard_render_page(backend->dash, page, &frame);
    if (rc < 0)
        return -1;

    if (page_count)
        *page_count = dashboard_page_count(backend->dash);

    memcpy(canvas, frame, needed);
    return 0;
}

static int dashboard_row_count(const xf_json_backend_t *backend)
{
    if (!backend)
        return 0;
    return backend->reg.count;
}

static int dashboard_row_view(const xf_json_backend_t *backend,
                              int idx,
                              xf_json_backend_row_view_t *view)
{
    const xf_reg_entry_t *entry;
    int                   page = 0;
    int                   page_on_page = 0;

    if (!backend || !view || idx < 0 || idx >= backend->reg.count)
        return -1;

    entry = &backend->reg.entries[idx];
    if (dashboard_comp_placement(backend->dash, entry->comp, &page, &page_on_page) < 0) {
        page = 0;
        page_on_page = 0;
    }

    view->id = entry->id;
    view->kind = entry->def->name;
    view->page = page;
    view->index = page_on_page;
    view->dirty = entry->comp ? entry->comp->dirty : 0;
    view->data = entry->data;
    view->encode = entry->def->encode;

    return 0;
}

static const xf_json_backend_ops_t g_dashboard_ops = {
    dashboard_destroy_backend,
    dashboard_backend_page_count,
    dashboard_row_add,
    dashboard_row_update,
    dashboard_row_remove,
    dashboard_row_move,
    dashboard_page_render_op,
    dashboard_row_count,
    dashboard_row_view,
};

xf_json_backend_t *xf_json_backend_dashboard_create(int width, int height, int padding)
{
    xf_json_backend_t *backend;

    if (width <= 0 || height <= 0 || padding < 0)
        return NULL;

    backend = calloc(1, sizeof(*backend));
    if (!backend)
        return NULL;

    backend->dash = dashboard_create(width, height, padding);
    if (!backend->dash) {
        free(backend);
        return NULL;
    }

    backend->ops = &g_dashboard_ops;
    backend->width = width;
    backend->height = height;
    return backend;
}

void xf_json_backend_destroy(xf_json_backend_t *backend)
{
    if (!backend)
        return;
    backend->ops->destroy(backend);
}

int xf_json_backend_page_count(const xf_json_backend_t *backend)
{
    if (!backend)
        return 0;
    return backend->ops->page_count(backend);
}

int xf_json_backend_row_add(xf_json_backend_t *backend,
                            const char *id,
                            xf_str_slice_t data_json,
                            const char **err_msg)
{
    return backend->ops->row_add(backend, id, data_json, err_msg);
}

int xf_json_backend_row_update(xf_json_backend_t *backend,
                               const char *id,
                               xf_str_slice_t data_json,
                               const char **err_msg)
{
    return backend->ops->row_update(backend, id, data_json, err_msg);
}

int xf_json_backend_row_remove(xf_json_backend_t *backend,
                               const char *id,
                               const char **err_msg)
{
    return backend->ops->row_remove(backend, id, err_msg);
}

int xf_json_backend_row_move(xf_json_backend_t *backend,
                             const char *id,
                             int dir,
                             const char **err_msg)
{
    return backend->ops->row_move(backend, id, dir, err_msg);
}

int xf_json_backend_page_render(xf_json_backend_t *backend,
                                int page,
                                uint8_t *canvas,
                                size_t canvas_cap,
                                int *page_count,
                                int *dx,
                                int *dy,
                                int *dw,
                                int *dh,
                                const char **err_msg)
{
    return backend->ops->page_render(backend, page, canvas, canvas_cap,
                                     page_count, dx, dy, dw, dh, err_msg);
}

int xf_json_backend_row_count(const xf_json_backend_t *backend)
{
    return backend->ops->row_count(backend);
}

int xf_json_backend_row_view(const xf_json_backend_t *backend,
                             int idx,
                             xf_json_backend_row_view_t *view)
{
    return backend->ops->row_view(backend, idx, view);
}

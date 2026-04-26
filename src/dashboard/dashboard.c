#include <stdlib.h>
#include <string.h>

#include "dashboard.h"
#include "components/comp_base.h"

/* Internal row record. y offset is computed on the fly during render
 * so that move/remove operations never need to reindex the array. */
typedef struct {
    xf_component_t **components; /* owned copy of the pointer array */
    int             *widths;     /* owned copy of caller's widths */
    int              count;
    int              height;
} row_t;

struct xf_dashboard {
    int      width;
    int      height;
    int      padding;
    uint8_t *framebuffer; /* width * height * 3, owned */
    row_t   *rows;        /* dynamic array, owned */
    int      row_count;
    int      row_cap;
};

xf_dashboard_t *dashboard_create(int width, int height, int padding)
{
    xf_dashboard_t *dash;

    if (width <= 0 || height <= 0 || padding < 0)
        return NULL;

    dash = calloc(1, sizeof(*dash));
    if (!dash)
        return NULL;

    dash->framebuffer = malloc((size_t)(width * height * 3));
    if (!dash->framebuffer) {
        free(dash);
        return NULL;
    }

    dash->width   = width;
    dash->height  = height;
    dash->padding = padding;
    return dash;
}

void dashboard_destroy(xf_dashboard_t *dash)
{
    int i;
    if (!dash)
        return;
    for (i = 0; i < dash->row_count; i++) {
        free(dash->rows[i].components);
        free(dash->rows[i].widths);
    }
    free(dash->rows);
    free(dash->framebuffer);
    free(dash);
}

int dashboard_add_row(xf_dashboard_t  *dash,
                      xf_component_t **components,
                      const int       *widths,
                      int              count,
                      int              height)
{
    int i, sum;
    row_t *row;

    if (!dash || !components || !widths || count <= 0 || height <= 0)
        return -1;

    sum = 0;
    for (i = 0; i < count; i++)
        sum += widths[i];
    if (sum != dash->width - 2 * dash->padding)
        return -1;

    if (dash->row_count == dash->row_cap) {
        int new_cap = dash->row_cap ? dash->row_cap * 2 : 4;
        row_t *tmp = realloc(dash->rows, (size_t)new_cap * sizeof(*dash->rows));
        if (!tmp)
            return -1;
        dash->rows    = tmp;
        dash->row_cap = new_cap;
    }

    row = &dash->rows[dash->row_count];

    row->components = malloc((size_t)count * sizeof(*row->components));
    if (!row->components)
        return -1;
    memcpy(row->components, components, (size_t)count * sizeof(*row->components));

    row->widths = malloc((size_t)count * sizeof(*row->widths));
    if (!row->widths) {
        free(row->components);
        return -1;
    }
    memcpy(row->widths, widths, (size_t)count * sizeof(*row->widths));

    row->count  = count;
    row->height = height;
    dash->row_count++;
    return 0;
}

int dashboard_add_full_row(xf_dashboard_t *dash,
                           xf_component_t *comp,
                           int             height)
{
    int w;
    if (!dash || !comp)
        return -1;
    w = dash->width - 2 * dash->padding;
    return dashboard_add_row(dash, &comp, &w, 1, height);
}

int dashboard_move_row_up(xf_dashboard_t *dash, int index)
{
    row_t tmp;
    if (!dash || index <= 0 || index >= dash->row_count)
        return -1;
    tmp                    = dash->rows[index - 1];
    dash->rows[index - 1]  = dash->rows[index];
    dash->rows[index]      = tmp;
    return 0;
}

int dashboard_move_row_down(xf_dashboard_t *dash, int index)
{
    row_t tmp;
    if (!dash || index < 0 || index >= dash->row_count - 1)
        return -1;
    tmp                    = dash->rows[index + 1];
    dash->rows[index + 1]  = dash->rows[index];
    dash->rows[index]      = tmp;
    return 0;
}

int dashboard_remove_row(xf_dashboard_t *dash, int index)
{
    if (!dash || index < 0 || index >= dash->row_count)
        return -1;

    free(dash->rows[index].components);
    free(dash->rows[index].widths);

    if (index < dash->row_count - 1) {
        memmove(&dash->rows[index],
                &dash->rows[index + 1],
                (size_t)(dash->row_count - index - 1) * sizeof(*dash->rows));
    }

    dash->row_count--;
    return 0;
}

/*
 * Single-pass page layout walker.
 * Returns total page count. If page_of_row / y_of_row are non-NULL they must
 * each hold at least dash->row_count elements and are filled with each row's
 * page index and y-offset within that page.
 */
static int page_layout(const xf_dashboard_t *dash, int *page_of_row, int *y_of_row)
{
    int r, y = 0, page = 0;
    int content_h = dash->height - 2 * dash->padding;
    for (r = 0; r < dash->row_count; r++) {
        if (y > 0 && y + dash->rows[r].height > content_h) {
            page++;
            y = 0;
        }
        if (page_of_row) page_of_row[r] = page;
        if (y_of_row)    y_of_row[r]    = y;
        y += dash->rows[r].height;
    }
    return dash->row_count == 0 ? 1 : page + 1;
}

static void render_row(xf_dashboard_t *dash, const row_t *row, int pad, int y)
{
    int c, ly, x = 0;

    for (c = 0; c < row->count; c++) {
        xf_component_t *comp = row->components[c];
        int w = row->widths[c];
        int h = row->height;
        uint8_t *sub = malloc((size_t)(w * h * 3));

        if (!sub) {
            x += w;
            continue;
        }

        comp_fetch(comp);
        comp_render(comp, sub, w, h);

        for (ly = 0; ly < h; ly++) {
            int fb_off  = ((pad + y + ly) * dash->width + pad + x) * 3;
            int sub_off = (ly * w) * 3;
            memcpy(dash->framebuffer + fb_off, sub + sub_off, (size_t)(w * 3));
        }

        free(sub);
        x += w;
    }
}

const uint8_t *dashboard_render_page(xf_dashboard_t *dash, int page)
{
    int r;
    int cur_page = 0, y = 0;

    if (!dash)
        return NULL;

    int pad = dash->padding;
    int content_h = dash->height - 2 * pad;

    xf_fill_rgb888(dash->framebuffer, dash->width, dash->height,
                   xf_get_theme()->background);

    /* Out-of-range page: return the cleared buffer without rendering. */
    if (page < 0 || page >= page_layout(dash, NULL, NULL))
        return dash->framebuffer;

    for (r = 0; r < dash->row_count; r++) {
        if (y > 0 && y + dash->rows[r].height > content_h) {
            cur_page++;
            y = 0;
        }

        if (cur_page > page)
            break;

        if (cur_page == page)
            render_row(dash, &dash->rows[r], pad, y);

        y += dash->rows[r].height;
    }

    return dash->framebuffer;
}

const uint8_t *dashboard_render(xf_dashboard_t *dash)
{
    return dashboard_render_page(dash, 0);
}

int dashboard_content_width(const xf_dashboard_t *dash)
{
    if (!dash) return 0;
    return dash->width - 2 * dash->padding;
}

int dashboard_page_count(xf_dashboard_t *dash)
{
    if (!dash)
        return 0;
    return page_layout(dash, NULL, NULL);
}

int dashboard_dirty_rect(xf_dashboard_t *dash, int page,
                         int *x, int *y, int *w, int *h)
{
    int r, c, found;
    int *page_of_row, *y_of_row;

    if (!dash)
        return -1;
    if (dash->row_count == 0)
        return 0;

    page_of_row = malloc((size_t)dash->row_count * sizeof(int));
    y_of_row    = malloc((size_t)dash->row_count * sizeof(int));
    if (!page_of_row || !y_of_row) {
        free(page_of_row);
        free(y_of_row);
        return -1;
    }
    page_layout(dash, page_of_row, y_of_row);

    found = 0;
    int xmin = 0, ymin = 0, xmax = 0, ymax = 0;

    for (r = 0; r < dash->row_count; r++) {
        if (page_of_row[r] != page)
            continue;

        row_t *row  = &dash->rows[r];
        int    ry   = dash->padding + y_of_row[r];
        int    cx   = dash->padding;

        for (c = 0; c < row->count; c++) {
            xf_component_t *comp = row->components[c];
            int cw = row->widths[c];
            int ch = row->height;

            if (comp->dirty) {
                if (!found) {
                    xmin = cx;       ymin = ry;
                    xmax = cx + cw;  ymax = ry + ch;
                    found = 1;
                } else {
                    if (cx       < xmin) xmin = cx;
                    if (cx + cw  > xmax) xmax = cx + cw;
                    if (ry       < ymin) ymin = ry;
                    if (ry + ch  > ymax) ymax = ry + ch;
                }
                comp->dirty = 0;
            }
            cx += cw;
        }
    }

    free(page_of_row);
    free(y_of_row);

    if (!found)
        return 0;

    *x = xmin;
    *y = ymin;
    *w = xmax - xmin;
    *h = ymax - ymin;
    return 1;
}

int dashboard_visit_dirty_rects(xf_dashboard_t *dash, int page,
                                xf_dirty_visitor_t visit, void *ctx)
{
    int r, c, count;
    int *page_of_row, *y_of_row;

    if (!dash)
        return -1;
    if (dash->row_count == 0)
        return 0;

    page_of_row = malloc((size_t)dash->row_count * sizeof(int));
    y_of_row    = malloc((size_t)dash->row_count * sizeof(int));
    if (!page_of_row || !y_of_row) {
        free(page_of_row);
        free(y_of_row);
        return -1;
    }
    page_layout(dash, page_of_row, y_of_row);

    count = 0;
    for (r = 0; r < dash->row_count; r++) {
        if (page_of_row[r] != page)
            continue;

        row_t *row = &dash->rows[r];
        int    ry  = dash->padding + y_of_row[r];
        int    cx  = dash->padding;

        for (c = 0; c < row->count; c++) {
            xf_component_t *comp = row->components[c];
            int cw = row->widths[c];
            int ch = row->height;

            if (comp->dirty) {
                visit(cx, ry, cw, ch, ctx);
                comp->dirty = 0;
                count++;
            }
            cx += cw;
        }
    }

    free(page_of_row);
    free(y_of_row);
    return count;
}

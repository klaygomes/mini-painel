#include <stdlib.h>
#include <string.h>

#include "dashboard.h"

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
    uint8_t *framebuffer; /* width * height * 3, owned */
    row_t   *rows;        /* dynamic array, owned */
    int      row_count;
    int      row_cap;
};

xf_dashboard_t *dashboard_create(int width, int height)
{
    xf_dashboard_t *dash;

    if (width <= 0 || height <= 0)
        return NULL;

    dash = calloc(1, sizeof(*dash));
    if (!dash)
        return NULL;

    dash->framebuffer = malloc((size_t)(width * height * 3));
    if (!dash->framebuffer) {
        free(dash);
        return NULL;
    }

    dash->width  = width;
    dash->height = height;
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
    if (sum != dash->width)
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
    w = dash->width;
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
    for (r = 0; r < dash->row_count; r++) {
        if (y > 0 && y + dash->rows[r].height > dash->height) {
            page++;
            y = 0;
        }
        if (page_of_row) page_of_row[r] = page;
        if (y_of_row)    y_of_row[r]    = y;
        y += dash->rows[r].height;
    }
    return dash->row_count == 0 ? 1 : page + 1;
}

const uint8_t *dashboard_render_page(xf_dashboard_t *dash, int page)
{
    int r, c, ly;
    int cur_page = 0, y = 0;

    if (!dash)
        return NULL;

    memset(dash->framebuffer, 0, (size_t)(dash->width * dash->height * 3));

    /* Out-of-range page: return the cleared buffer without rendering. */
    if (page < 0 || page >= page_layout(dash, NULL, NULL))
        return dash->framebuffer;

    for (r = 0; r < dash->row_count; r++) {
        if (y > 0 && y + dash->rows[r].height > dash->height) {
            cur_page++;
            y = 0;
        }

        if (cur_page > page)
            break;

        if (cur_page == page) {
            row_t *row = &dash->rows[r];
            int x = 0;

            for (c = 0; c < row->count; c++) {
                xf_component_t *comp = row->components[c];
                int w = row->widths[c];
                int h = row->height;
                uint8_t *sub = calloc((size_t)(w * h * 3), 1);

                if (!sub) {
                    x += w;
                    continue;
                }

                if (comp->fetch)
                    comp->fetch(comp);

                comp->render(comp, sub, w, h);

                for (ly = 0; ly < h; ly++) {
                    int fb_off  = ((y + ly) * dash->width + x) * 3;
                    int sub_off = (ly * w) * 3;
                    memcpy(dash->framebuffer + fb_off, sub + sub_off, (size_t)(w * 3));
                }

                free(sub);
                x += w;
            }
        }

        y += dash->rows[r].height;
    }

    return dash->framebuffer;
}

const uint8_t *dashboard_render(xf_dashboard_t *dash)
{
    return dashboard_render_page(dash, 0);
}

int dashboard_page_count(xf_dashboard_t *dash)
{
    if (!dash)
        return 0;
    return page_layout(dash, NULL, NULL);
}

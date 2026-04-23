#ifndef DASHBOARD_H
#define DASHBOARD_H

#include <stdint.h>

/*
 * xf_component_t — a fetch/render pair with a caller-owned context pointer.
 *
 * Transparent struct: stack-allocate or embed it. The dashboard stores
 * a pointer to it; the component lifetime is the caller's responsibility.
 */
typedef struct xf_component xf_component_t;
struct xf_component {
    /*
     * Called once per frame before render() to refresh ctx with live data.
     * May be NULL if no data fetching is needed.
     * A non-zero return is non-fatal: render() still runs with the existing ctx.
     */
    int  (*fetch)(xf_component_t *self);

    /*
     * Draws into buf: a zeroed RGB888 region of width * height * 3 bytes.
     * The top-left corner of buf maps to the component's position on screen.
     * Required — must not be NULL.
     */
    void (*render)(xf_component_t *self, uint8_t *buf, int width, int height);

    /* Caller-owned context passed into fetch() and render() via self->ctx.
     * The dashboard never reads or frees this pointer. */
    void *ctx;

    /* Set to non-zero by the caller to signal that the component's output has
     * changed since the last display update. dashboard_dirty_rect() reads and
     * clears this flag; the dashboard itself never modifies it. */
    int dirty;
};

/*
 * Component initialiser macros — use these instead of bare struct literals.
 *
 *   XF_COMPONENT(render_fn)
 *     Pure rendering, no data. Use for static visuals (logos, separators, etc.)
 *
 *   XF_COMPONENT_DATA(render_fn, ctx)
 *     Rendering driven by a caller-owned context pointer. fetch() is never
 *     called; the caller updates ctx between frames as needed.
 *
 *   XF_COMPONENT_LIVE(fetch_fn, render_fn, ctx)
 *     fetch() is called every frame to refresh ctx before render().
 *     Use for live data (clock, CPU stats, sensor readings, etc.)
 */
#define XF_COMPONENT(render_fn) \
    { NULL, (render_fn), NULL, 0 }

#define XF_COMPONENT_DATA(render_fn, ctx) \
    { NULL, (render_fn), (ctx), 0 }

#define XF_COMPONENT_LIVE(fetch_fn, render_fn, ctx) \
    { (fetch_fn), (render_fn), (ctx), 0 }

/* Opaque dashboard handle. */
typedef struct xf_dashboard xf_dashboard_t;

/*
 * Allocate a dashboard for a display of the given pixel dimensions.
 * padding is added on all four sides; the usable content area is
 * (width - 2*padding) × (height - 2*padding). Pass 0 for no padding.
 * Returns NULL on allocation failure.
 */
xf_dashboard_t *dashboard_create(int width, int height, int padding);

/*
 * Free the dashboard and its internal framebuffer.
 * Components pointed to by rows are not freed. Safe to call with NULL.
 */
void dashboard_destroy(xf_dashboard_t *dash);

/*
 * Append a row to the bottom of the dashboard.
 *
 *   components  array of component pointers; the dashboard copies this array.
 *   widths      pixel width of each component; must sum to the content width (dashboard width - 2*padding).
 *   count       number of components in this row; must be >= 1.
 *   height      row height in pixels; must be >= 1.
 *
 * Returns 0 on success, -1 on invalid arguments or allocation failure.
 */
int dashboard_add_row(xf_dashboard_t  *dash,
                      xf_component_t **components,
                      const int       *widths,
                      int              count,
                      int              height);

/*
 * Append a single component that spans the full dashboard width.
 * Equivalent to dashboard_add_row with count=1 and widths={content_width}.
 * Returns 0 on success, -1 on error.
 */
int dashboard_add_full_row(xf_dashboard_t *dash,
                           xf_component_t *comp,
                           int             height);

/*
 * Swap the row at index with the row above it (index - 1).
 * Returns 0 on success, -1 if index is 0 or out of bounds.
 */
int dashboard_move_row_up(xf_dashboard_t *dash, int index);

/*
 * Swap the row at index with the row below it (index + 1).
 * Returns 0 on success, -1 if index is the last row or out of bounds.
 */
int dashboard_move_row_down(xf_dashboard_t *dash, int index);

/*
 * Remove the row at index and free its internal bookkeeping.
 * Components themselves are not freed.
 * Returns 0 on success, -1 if index is out of bounds.
 */
int dashboard_remove_row(xf_dashboard_t *dash, int index);

/*
 * Render page 0 into the internal framebuffer. Equivalent to
 * dashboard_render_page(dash, 0). Rows that overflow the display height are
 * excluded and appear on subsequent pages instead.
 *
 * Returns a pointer to the internal RGB888 buffer (width * height * 3 bytes).
 * The pointer remains valid until the next render call or dashboard_destroy().
 * Returns NULL if dash is NULL.
 */
const uint8_t *dashboard_render(xf_dashboard_t *dash);

/*
 * Returns the usable content width (display width minus horizontal padding).
 * Use this when building widths[] arrays for dashboard_add_row.
 */
int dashboard_content_width(const xf_dashboard_t *dash);

/*
 * Returns the number of pages the current row list produces.
 * A new page begins whenever a row would overflow the bottom of the display.
 * Always returns >= 1 for a valid dashboard. Returns 0 for NULL.
 */
int dashboard_page_count(xf_dashboard_t *dash);

/*
 * Render a specific page into the internal framebuffer.
 * Rows on earlier pages are skipped; rows on later pages are excluded.
 * An out-of-range page index clears the buffer to black (returns non-NULL).
 * Returns a pointer to the internal RGB888 buffer, or NULL if dash is NULL.
 */
const uint8_t *dashboard_render_page(xf_dashboard_t *dash, int page);

/*
 * Computes the bounding rectangle (in framebuffer coordinates, padding
 * included) that covers every component on the given page whose dirty flag
 * is set, then clears those flags.
 *
 * Returns 1 and fills x/y/w/h if at least one dirty component was found.
 * Returns 0 if no component was dirty (x/y/w/h are left unchanged).
 * Returns -1 if dash is NULL.
 */
int dashboard_dirty_rect(xf_dashboard_t *dash, int page,
                         int *x, int *y, int *w, int *h);

/*
 * Calls visit(x, y, w, h, ctx) once per dirty component on the given page,
 * then clears all dirty flags. Each call covers exactly the component's own
 * pixel rectangle rather than a union of all dirty areas, so unchanged
 * regions between dirty components are never sent.
 *
 * Returns the number of dirty components found (0 if none), -1 if dash is NULL.
 */
typedef void (*xf_dirty_visitor_t)(int x, int y, int w, int h, void *ctx);
int dashboard_visit_dirty_rects(xf_dashboard_t *dash, int page,
                                xf_dirty_visitor_t visit, void *ctx);

#endif /* DASHBOARD_H */

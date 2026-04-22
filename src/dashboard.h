#ifndef DASHBOARD_H
#define DASHBOARD_H

#include <stdint.h>

/*
 * xf_component_t — a fetch/render pair with a caller-owned payload.
 *
 * Transparent struct: stack-allocate or embed it. The dashboard stores
 * a pointer to it; the component lifetime is the caller's responsibility.
 */
typedef struct xf_component xf_component_t;
struct xf_component {
    /*
     * Called once per frame before render() to refresh payload with live data.
     * May be NULL if no data fetching is needed.
     * A non-zero return is non-fatal: render() still runs with the existing payload.
     */
    int  (*fetch)(xf_component_t *self);

    /*
     * Draws into buf: a zeroed RGB888 region of width * height * 3 bytes.
     * The top-left corner of buf maps to the component's position on screen.
     * Required — must not be NULL.
     */
    void (*render)(xf_component_t *self, uint8_t *buf, int width, int height);

    /* Component-specific data. Not managed by the dashboard. */
    void *payload;
};

/* Opaque dashboard handle. */
typedef struct xf_dashboard xf_dashboard_t;

/*
 * Allocate a dashboard for a display of the given pixel dimensions.
 * Returns NULL on allocation failure.
 */
xf_dashboard_t *dashboard_create(int width, int height);

/*
 * Free the dashboard and its internal framebuffer.
 * Components pointed to by rows are not freed. Safe to call with NULL.
 */
void dashboard_destroy(xf_dashboard_t *dash);

/*
 * Append a row to the bottom of the dashboard.
 *
 *   components  array of component pointers; the dashboard copies this array.
 *   widths      pixel width of each component; must sum to the dashboard width.
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
 * Equivalent to dashboard_add_row with count=1 and widths={dash->width}.
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

#endif /* DASHBOARD_H */

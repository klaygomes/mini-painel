# Dashboard API Reference

The dashboard module sits on top of `panel_display_bitmap()` and lets you compose a screen from independent, reusable components without managing pixel math yourself.

You define rows. Each row holds one or more components side by side. When you call `dashboard_render()` you get back a single RGB888 framebuffer that you can hand straight to `panel_display_bitmap()`.

---

## Types

### `xf_component_t`

```c
typedef struct xf_component xf_component_t;
struct xf_component {
    int  (*fetch)(xf_component_t *self);
    void (*render)(xf_component_t *self, uint8_t *buf, int width, int height);
    void *payload;
};
```

A component is a plain struct — stack-allocate it or embed it inside a larger struct. The dashboard stores a pointer to it; you own the lifetime.

| Field | Purpose |
|---|---|
| `fetch` | Called once per frame before `render` to refresh `payload` with live data. May be `NULL`. A non-zero return is non-fatal: `render` still runs. |
| `render` | Draws the component into `buf`. `buf` is a zeroed `width × height × 3` byte region; fill it with RGB888 pixels. Must not be `NULL`. |
| `payload` | Anything you need — a struct with live sensor readings, an image pointer, etc. Not managed by the dashboard. |

### `xf_dashboard_t`

An opaque handle. Create one with `dashboard_create()`, destroy it with `dashboard_destroy()`.

---

## Component initialiser macros

Use these instead of bare struct literals — they make the component's role clear at a glance.

| Macro | When to use |
|---|---|
| `XF_COMPONENT(render_fn)` | Pure static rendering; no data, no fetch. Logos, separators, solid fills. |
| `XF_COMPONENT_DATA(render_fn, payload_ptr)` | Pre-loaded data that the render function reads. Caller updates payload between frames as needed. |
| `XF_COMPONENT_LIVE(fetch_fn, render_fn, payload_ptr)` | Live data: `fetch()` is called every frame to refresh payload before `render()`. Clock, sensor readings, CPU stats, etc. |

```c
/* Static visual — no data needed */
xf_component_t logo = XF_COMPONENT(render_logo);

/* Static data loaded once before the dashboard loop */
xf_component_t label = XF_COMPONENT_DATA(render_text, &my_text);

/* Live data refreshed every frame */
xf_component_t clock = XF_COMPONENT_LIVE(fetch_time, render_clock, &time_payload);
```

---

## Functions

### `dashboard_create`

```c
xf_dashboard_t *dashboard_create(int width, int height);
```

Allocates a dashboard for a display of `width × height` pixels and its internal RGB888 framebuffer. Returns `NULL` on allocation failure.

### `dashboard_destroy`

```c
void dashboard_destroy(xf_dashboard_t *dash);
```

Frees the dashboard and its framebuffer. Components pointed to by rows are **not** freed — you own them. Safe to call with `NULL`.

### `dashboard_add_row`

```c
int dashboard_add_row(xf_dashboard_t  *dash,
                      xf_component_t **components,
                      const int       *widths,
                      int              count,
                      int              height);
```

Appends a row to the bottom of the dashboard.

| Parameter | Meaning |
|---|---|
| `components` | Array of component pointers. The dashboard copies the array; you may free it after this call. |
| `widths` | Pixel width of each component. Must sum exactly to the dashboard width. |
| `count` | Number of components. Must be ≥ 1. |
| `height` | Row height in pixels. Must be ≥ 1. |

Returns `0` on success, `-1` on invalid arguments or allocation failure.

### `dashboard_add_full_row`

```c
int dashboard_add_full_row(xf_dashboard_t *dash,
                           xf_component_t *comp,
                           int             height);
```

Convenience wrapper for a single component that spans the full dashboard width. Equivalent to calling `dashboard_add_row` with `count=1` and `widths={dash->width}`.

Returns `0` on success, `-1` on error.

### `dashboard_move_row_up`

```c
int dashboard_move_row_up(xf_dashboard_t *dash, int index);
```

Swaps the row at `index` with the row above it (`index − 1`). The change takes effect on the next `dashboard_render()`.

Returns `0` on success, `-1` if `index` is `0` (already the first row) or out of bounds.

### `dashboard_move_row_down`

```c
int dashboard_move_row_down(xf_dashboard_t *dash, int index);
```

Swaps the row at `index` with the row below it (`index + 1`).

Returns `0` on success, `-1` if `index` is the last row or out of bounds.

### `dashboard_remove_row`

```c
int dashboard_remove_row(xf_dashboard_t *dash, int index);
```

Removes the row at `index` and frees its internal bookkeeping. Components are not freed. Rows below the removed one shift up by one.

Returns `0` on success, `-1` if `index` is out of bounds.

### `dashboard_render`

```c
const uint8_t *dashboard_render(xf_dashboard_t *dash);
```

Renders page 0 into the internal framebuffer. Equivalent to `dashboard_render_page(dash, 0)`. Rows that overflow the display height are excluded and appear on subsequent pages.

Returns a pointer to the internal RGB888 buffer (`width × height × 3` bytes). The pointer is valid until the next render call or `dashboard_destroy()`. Returns `NULL` if `dash` is `NULL`.

### `dashboard_page_count`

```c
int dashboard_page_count(xf_dashboard_t *dash);
```

Returns the number of pages the current row list produces. A new page begins whenever a row would overflow the bottom of the display. Always returns ≥ 1 for a valid dashboard. Returns 0 for `NULL`.

### `dashboard_render_page`

```c
const uint8_t *dashboard_render_page(xf_dashboard_t *dash, int page);
```

Renders a specific page into the internal framebuffer. Rows on earlier pages are skipped; rows on later pages are excluded. An out-of-range `page` index clears the buffer to black and returns a non-`NULL` pointer. Returns `NULL` if `dash` is `NULL`.

Typical usage for a multi-page dashboard:

```c
int pages = dashboard_page_count(dash);
int p;
for (p = 0; p < pages; p++) {
    const uint8_t *frame = dashboard_render_page(dash, p);
    panel_display_bitmap(dev, 0, 0, 320, 480, frame);
    /* wait for user input to advance, or cycle automatically */
}
```

---

## Worked example

```c
#include "panel.h"
#include "dashboard.h"

/* A component that draws a solid colour. */
typedef struct { uint8_t r, g, b; } solid_payload_t;

static void render_solid(xf_component_t *self, uint8_t *buf, int w, int h)
{
    solid_payload_t *p = self->payload;
    int i;
    for (i = 0; i < w * h; i++) {
        buf[i*3+0] = p->r;
        buf[i*3+1] = p->g;
        buf[i*3+2] = p->b;
    }
}

int main(void)
{
    solid_payload_t header_data = {0x1A, 0x6B, 0xC8}; /* blue  */
    solid_payload_t left_data   = {0xC0, 0x20, 0x20}; /* red   */
    solid_payload_t right_data  = {0x20, 0xA0, 0x40}; /* green */

    xf_component_t header = XF_COMPONENT_DATA(render_solid, &header_data);
    xf_component_t left   = XF_COMPONENT_DATA(render_solid, &left_data);
    xf_component_t right  = XF_COMPONENT_DATA(render_solid, &right_data);

    xf_component_t *body_comps[]  = {&left, &right};
    int             body_widths[] = {160, 160};

    xf_dashboard_t *dash = dashboard_create(320, 480);
    dashboard_add_full_row(dash, &header, 60);
    dashboard_add_row(dash, body_comps, body_widths, 2, 420);

    xf_device_t *dev = panel_open_auto();
    panel_set_orientation(dev, XF_ORIENT_PORTRAIT);
    panel_set_brightness(dev, 80);

    const uint8_t *frame = dashboard_render(dash);
    panel_display_bitmap(dev, 0, 0, 320, 480, frame);

    panel_close(dev);
    dashboard_destroy(dash);
    return 0;
}
```

---

## Using a graphics library inside a component

The `render()` callback receives a plain `uint8_t *buf` of `width × height × 3` bytes. You can use any library that can write pixels into that buffer.

**stb_image** — load an image in `fetch()`, blit or scale it in `render()`:

```c
static int fetch_image(xf_component_t *self)
{
    img_payload_t *p = self->payload;
    int w, h, n;
    p->pixels = stbi_load(p->path, &w, &h, &n, 3); /* RGB888 */
    return p->pixels ? 0 : -1;
}
```

**Cairo** — draw vector graphics in `render()` then pack to RGB888:

```c
static void render_cairo(xf_component_t *self, uint8_t *buf, int w, int h)
{
    cairo_surface_t *surf = cairo_image_surface_create(CAIRO_FORMAT_RGB24, w, h);
    cairo_t *cr = cairo_create(surf);

    /* ... draw with Cairo ... */

    /* Pack XRGB32 → RGB888 */
    unsigned char *src = cairo_image_surface_get_data(surf);
    int stride = cairo_image_surface_get_stride(surf);
    int x, y;
    for (y = 0; y < h; y++)
        for (x = 0; x < w; x++) {
            uint32_t px;
            memcpy(&px, src + y * stride + x * 4, 4);
            buf[(y * w + x) * 3 + 0] = (px >> 16) & 0xFF; /* R */
            buf[(y * w + x) * 3 + 1] = (px >>  8) & 0xFF; /* G */
            buf[(y * w + x) * 3 + 2] =  px        & 0xFF; /* B */
        }

    cairo_destroy(cr);
    cairo_surface_destroy(surf);
}
```

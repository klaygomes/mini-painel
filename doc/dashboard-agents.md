# Dashboard module — agent context

## What this module is

A row-based immediate-mode layout engine. It composes a screen from independent
components without retaining any render state between frames. Every
`dashboard_render()` call clears the framebuffer and redraws from scratch.

It has **no graphics dependency** of its own. A component's `render()` callback
receives a plain zeroed RGB888 sub-buffer and fills it however it likes —
raw pixel writes, stb_image, Cairo, or anything else.

It does **not** depend on `panel.h`. It produces a `const uint8_t *` RGB888
buffer; what the caller does with that buffer (send to hardware, preview in SDL,
save as PNG) is outside its scope.

## File layout

| File | Responsibility |
|---|---|
| `src/dashboard.h` | Public API: `xf_component_t`, `xf_dashboard_t`, all function declarations |
| `src/dashboard.c` | Implementation: `row_t` internals, create/destroy, add/move/remove, render loop |
| `tests/test_dashboard.c` | All unit tests (no hardware, no serial, no panel dependency) |
| `doc/dashboard.md` | Human-focused API reference and worked examples |

## Architecture

### `row_t` (internal, `dashboard.c` only)

```c
typedef struct {
    xf_component_t **components; /* owned copy of pointer array */
    int             *widths;     /* owned copy of caller's widths */
    int              count;
    int              height;
} row_t;
```

The `xf_dashboard` struct holds a dynamic array of `row_t` (doubling strategy,
initial cap 4). The `rows` array and both pointer arrays inside each `row_t`
are owned by the dashboard. Component structs themselves are **not** owned.

### y-offset computation

There is no precomputed `y` field in `row_t`. `dashboard_render()` accumulates
`y` by iterating the rows array in order:

```c
y = 0;
for each row:
    blit at y
    y += row->height
```

This is intentional: move/remove operations become O(1) array swaps or
`memmove` with no reindex step.

### Sub-buffer blit

For each component, `dashboard_render()`:
1. `calloc`s a `w × h × 3` byte sub-buffer (zeroed)
2. Calls `comp->fetch()` if set
3. Calls `comp->render()` into the sub-buffer
4. Copies it row by row into the framebuffer at the correct `(x, y)` offset:
   `fb_offset = ((y + ly) * dash->width + x) * 3`
5. Frees the sub-buffer

## Testing rules

Tests are in `tests/test_dashboard.c` and use Unity (vendored). No hardware,
no serial, no `panel.h` dependency.

**What to import**: `dashboard.h` and `vendor/unity.h` only.

**What never to do**: access `row_t` or `xf_dashboard` fields directly;
the internal layout is an implementation detail.

### Pixel-colour pattern

Create a component whose `render()` fills its entire region with a single
known colour, add it to a dashboard, call `dashboard_render()`, and read
specific pixel coordinates in the returned buffer:

```c
static void render_red(xf_component_t *self, uint8_t *buf, int w, int h)
{
    int i;
    (void)self;
    for (i = 0; i < w * h; i++) { buf[i*3]=0xFF; buf[i*3+1]=0x00; buf[i*3+2]=0x00; }
}

/* In the test: */
xf_component_t comp = {NULL, render_red, NULL};
dashboard_add_full_row(dash, &comp, H);
const uint8_t *fb = dashboard_render(dash);

/* pixel at (x, y), channel ch: fb[(y * W + x) * 3 + ch] */
TEST_ASSERT_EQUAL_UINT8(0xFF, fb[0 * 3 + 0]); /* pixel (0,0) red channel */
```

Use multiple differently-coloured components to assert that each occupies
the correct region (left vs right column, top vs bottom row).

### Lifecycle flags

Use module-level static variables to track callback invocation. Reset them
in `setUp()`:

```c
static int fetch_ran = 0;
static int render_saw_fetch = 0;

static int fetch_set_flag(xf_component_t *self) { (void)self; fetch_ran = 1; return 0; }
static void render_check_flag(xf_component_t *self, uint8_t *buf, int w, int h) {
    (void)self; (void)buf; (void)w; (void)h;
    render_saw_fetch = fetch_ran;
}

void setUp(void) { fetch_ran = 0; render_saw_fetch = 0; }
```

This lets you assert call order and that `render()` still runs when `fetch()`
returns an error or is `NULL` — without any knowledge of internals.

## What not to do

- **Do not add a `y` cache to `row_t`** — it would need reindexing on every
  move or remove.
- **Do not couple `dashboard.c` to `panel.h`** — the module must stay
  output-agnostic.
- **Do not free component structs in `dashboard_destroy()`** — components are
  caller-owned; the dashboard only borrows pointers.
- **Do not add a persistence layer** — this is immediate mode. If a component
  needs state, it stores it in its own `payload`.
- **Do not pre-allocate a permanent sub-buffer** per component — components
  have varying sizes and the current per-render `calloc`/`free` keeps the API
  simple without measurable overhead at display frame rates.

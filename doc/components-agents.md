# Components module — agent context

## What this module is

A collection of pre-built Cairo-rendered dashboard widgets that write directly
into the RGB888 sub-buffer that `dashboard.h` provides to every component's
`render()` callback.

All components are engine-agnostic at the API surface: component headers include
only `dashboard.h` and `draw.h`; Cairo is isolated entirely in `draw.c`. Swapping
the rendering backend requires replacing `draw.c` — no component source changes.

## Build prerequisite: Cairo

Cairo is a required system dependency. It is **not** vendored.

Install before the first `cmake` configure:

```sh
# macOS
brew install cairo

# Debian / Ubuntu
sudo apt-get install libcairo2-dev

# Fedora
sudo dnf install cairo-devel
```

CMake detects it via `pkg_check_modules(CAIRO REQUIRED cairo)`. If configure
fails with "Package 'cairo' not found", Cairo is not on the pkg-config search
path — installing it as above fixes this.

## File layout

Run `tree src/draw src/theme src/components` to see the current layout — do not restate it here, it drifts.

Roles you should know without reading the tree:

- `src/draw/draw.h` — engine-agnostic draw API. **The only file that includes `cairo.h` is `src/draw/draw.c`.**
- `src/draw/layout.h` — `LAY_*` row-height and gap constants.
- `src/theme/theme.h` — `xf_rgba_t`, `xf_theme_t`, `xf_set_theme` / `xf_get_theme`, `FONT_*`, `WEIGHT_*`.
- `src/components/comp_base.h` — convenience include (`dashboard.h` + `draw.h` + `layout.h`).
- `src/components/comp_*.{h,c}` — one widget per pair. Each header defines `COMP_<NAME>_HEIGHT`.
- `src/gfx/*` — reusable drawing primitives (avatar, pill, chip, dot, progress bar, etc.). Components include only the specific `gfx/<name>.h` they use.

Authoritative heights live in each component header as `COMP_<NAME>_HEIGHT`. Read those macros directly — never restate the numbers in docs (they drift).

`xf_components` is a static library. It links Cairo as PUBLIC so that any
target linking `xf_components` automatically pulls in Cairo at link time —
static libraries do not embed their dependencies.

## draw.h — the only header components include for drawing

`src/draw/draw.h` has zero Cairo types in its public surface. It includes `theme.h` and exposes:

- `xf_rgba_t` — normalized RGBA (double channels in [0.0, 1.0]) *(from theme.h)*
- `XF_RGB(0xRRGGBB)` / `XF_RGBA(0xRRGGBB, alpha)` macros *(from theme.h)*
- `xf_theme_t` — full colour and typography palette *(from theme.h)*
- `xf_theme` — default light theme *(from theme.h)*
- `xf_set_theme` / `xf_get_theme` — module-level theme pointer *(from theme.h)*
- `xf_draw_ctx_t` — opaque draw context (components never inspect it)
- Shape and text draw functions: `xf_draw_fill_round_rect`, `xf_draw_circle`, `xf_draw_text`, etc.
- Path API: `xf_draw_begin_path`, `xf_draw_move_to`, `xf_draw_line_to`, `xf_draw_close_path`, `xf_draw_fill`, `xf_draw_stroke`
- `xf_render(buf, w, h, fn, user_data)` — creates a Cairo ARGB32 surface, calls `fn`, converts to RGB888 into `buf`, destroys the surface. Returns `int` (`xf_result_t`); a non-zero return means the conversion failed and `buf` is in an undefined state. Always propagate this value — every component's `render` callback must return the `int` from `xf_render` directly. Error codes are defined in `src/status.h`.

### Cairo include path gotcha

`pkg_check_modules` appends `/cairo` to the include path it reports
(e.g. `/opt/homebrew/include/cairo`). This means the correct include inside
`draw.c` is:

```c
#include <cairo.h>    /* correct — pkg-config already included the cairo/ dir */
```

**Not**:

```c
#include <cairo/cairo.h>  /* wrong with pkg_check_modules — double cairo/ */
```

If you see `'cairo/cairo.h' file not found`, change the include to `<cairo.h>`.

## Component pattern

There are three factory macros, each expanding to a brace-initialiser. Always
assign to a local variable — a bare `return XF_COMPONENT*(...)` is invalid C99.

| Macro | Use case |
|---|---|
| `XF_COMPONENT(render_fn)` | No data — static visuals (spacer, divider) |
| `XF_COMPONENT_DATA(render_fn, ctx)` | Caller-owned data; `fetch` is never called |
| `XF_COMPONENT_LIVE(fetch_fn, render_fn, ctx)` | Live data; `fetch` runs every frame before `render` |

### Header (data component)

```c
#pragma once
#include "comp_base.h"

#define COMP_FOO_HEIGHT <N>

typedef struct {
    char      label[32];
    xf_rgba_t dot;        /* caller assigns from theme at creation time */
} comp_foo_data_t;

xf_component_t comp_foo_create(comp_foo_data_t *data);
```

### Implementation (data component)

Include only the specific gfx headers the component needs.

```c
#include "comp_foo.h"
#include "gfx/dot.h"      /* include only what is used */

static void draw(xf_draw_ctx_t *ctx, void *user_data)
{
    const xf_theme_t      *t = xf_get_theme();
    const comp_foo_data_t *d = user_data;

    xf_draw_text(ctx, d->label, LAY_PAD_X, 13.0, &(xf_text_opts_t){
        .size = FONT_LG, .weight = WEIGHT_NORMAL, .color = t->text_primary
    });
    xf_gfx_dot(ctx, 4.0, 8.0, 3.0, d->dot);
}

static int render(xf_component_t *self, uint8_t *buf, int w, int h)
{
    return xf_render(buf, w, h, draw, self->ctx);
}

xf_component_t comp_foo_create(comp_foo_data_t *data)
{
    xf_component_t c = XF_COMPONENT_DATA(render, data);
    return c;
}
```

### Implementation (live data component)

`fetch` refreshes the context pointer before every frame. A non-zero return is
non-fatal — `render` still runs with the existing data.

```c
#include "comp_foo.h"

static int fetch(xf_component_t *self)
{
    comp_foo_data_t *d = self->ctx;
    /* populate d from live source */
    return 0;
}

static void draw(xf_draw_ctx_t *ctx, void *user_data) { /* same as above */ }

static int render(xf_component_t *self, uint8_t *buf, int w, int h)
{
    return xf_render(buf, w, h, draw, self->ctx);
}

xf_component_t comp_foo_create(comp_foo_data_t *data)
{
    xf_component_t c = XF_COMPONENT_LIVE(fetch, render, data);
    return c;
}
```

### Implementation (no-data component)

```c
#include "comp_spacer.h"

static void draw(xf_draw_ctx_t *ctx, void *user_data)
{
    (void)ctx; (void)user_data;
}

static void render(xf_component_t *self, uint8_t *buf, int w, int h)
{
    (void)self;
    xf_render(buf, w, h, draw, NULL);
}

xf_component_t comp_spacer_create(void)
{
    xf_component_t c = XF_COMPONENT(render);
    return c;
}
```

### NULL in component files

`comp_base.h` → `draw.h` → `theme.h` → `<stddef.h>`, so `NULL` is always
available for the `XF_COMPONENT*` macros without an explicit include.

## Status enums

Some components expose a typed status field that controls indicator colours without passing raw `xf_rgba_t` values.

### `comp_deploy_status_t` (`comp_deploy.h`)

| Value | Integer | Dot colour (theme field) | Meaning |
|---|---|---|---|
| `COMP_DEPLOY_STATUS_BUILDING` | `0` | `t->warning` (yellow) | In progress |
| `COMP_DEPLOY_STATUS_SUCCESS` | `1` | `t->success` (green) | Passed |
| `COMP_DEPLOY_STATUS_FAILED` | `2` | `t->danger` (red) | Failed |

The component selects the dot colour at render time from the active theme — no hex literal appears in the component source. Assign `status` when creating or updating the data struct:

```c
comp_deploy_data_t d = {
    .branch   = "main@a3f2c1",
    .time_ago = "8m",
    .label    = "prod",
    .status   = COMP_DEPLOY_STATUS_SUCCESS,
};
```

Via JSON API, pass the integer value in `"status"`. Absent `"status"` defaults to `COMP_DEPLOY_STATUS_BUILDING` (`0`).

## Colour rule

**No colour literal may appear in any component source file.** Every colour is a
field dereference on the active theme:

```c
/* correct */
xf_draw_circle(ctx, cx, cy, r, t->danger);

/* wrong — literal hex in component code */
xf_draw_circle(ctx, cx, cy, r, (xf_rgba_t)XF_RGB(0xE24B4A));
```

Per-row colours (outage severity, alert severity, etc.) are stored as `xf_rgba_t`
fields in the data struct. The caller assigns theme values at creation time:

```c
d.rows[0].dot    = t->danger;
d.rows[0].row_bg = t->danger_bg;
```

The render function applies them without any switch-on-severity logic.

## Theme

`xf_set_theme` must be called before the first render. Omitting it falls back
to `xf_theme` (the call to `xf_get_theme` inside `draw.c` returns the
default if the pointer was never set).

### Token naming tiers

Every semantic role in `xf_theme_t` (`danger`, `warning`, `success`, `info`, `accent`) exposes tiers with these suffixes. Read `src/theme/theme.h` for the full field list — do not restate it here.

| Suffix | Role |
|--------|------|
| *(none)* | Primary indicator: icons, dots, borders |
| `_bg` | Subtle tinted background: card, row highlight |
| `_fg` | Body text on `_bg` |
| `_badge_bg` | Opaque badge/tag background |
| `_badge_fg` | Text inside a badge or tag |
| `_emphasis` | Darkest shade: high-contrast labels or headings |
| `_subtle` | Semi-transparent area fill: charts, sparklines |

Forbidden suffixes (these encode widget shapes or tonal adjectives, not roles): `_pill_*`, `_chip_*`, `_title_*`, `_bar`, `_dark`, `_fill`.

All four main roles (`danger`, `warning`, `success`, `info`) must expose the same set of tiers. Adding a tier to one role requires adding it to the others in the same commit.

Special tokens that do not follow the role+tier pattern:

- `on_color` — foreground drawn on any filled/coloured surface (checkmark strokes, avatar initials, dot rings). Use this wherever you would otherwise reach for a hardcoded white.
- `status_offline` — avatar background for offline or unknown presence.
- `caution` — distinct orange tone between `warning` and `danger`.
- `chart_accent` — primary bar or progress fill for charts with no semantic state.

The text scale is ordered highest to lowest contrast: `text_primary` → `text_secondary` → `text_muted` → `text_subtle`. Do not insert new steps between existing ones; add at the end or rename.

Enforcement: `doc/theme-tokens.instructions.md` (for `src/theme/**`) and `doc/component-token-usage.instructions.md` (for `src/components/**`, `src/gfx/**`, `src/draw/**`).

Call it once at startup:

```c
xf_set_theme(&xf_theme);
```

To test a custom theme, assign a modified copy and set it:

```c
xf_theme_t dark = xf_theme;
dark.text_primary = (xf_rgba_t)XF_RGB(0xEEEEEE);
xf_set_theme(&dark);
/* ... render ... */
xf_set_theme(&xf_theme);  /* restore */
```

## Testing

Tests live in `tests/test_components.c`.

### Pattern

```c
void setUp(void) { xf_set_theme(&xf_theme); }

static void test_metrics(void)
{
    comp_metrics_data_t d = {0};
    d.count = 2;
    snprintf(d.cards[0].label, sizeof(d.cards[0].label), "UPTIME");
    snprintf(d.cards[0].value, sizeof(d.cards[0].value), "99.94%%");

    xf_component_t comp = comp_metrics_create(&d);

    xf_dashboard_t *dash = dashboard_create(320, COMP_METRICS_HEIGHT, 0);
    TEST_ASSERT_EQUAL_INT(0, dashboard_add_full_row(dash, &comp, COMP_METRICS_HEIGHT));

    const uint8_t *buf = dashboard_render(dash);
    TEST_ASSERT_NOT_NULL(buf);
    /* At least one non-zero byte confirms the component drew something */
    int has_content = 0;
    for (int i = 0; i < 320 * COMP_METRICS_HEIGHT * 3; i++)
        if (buf[i]) { has_content = 1; break; }
    TEST_ASSERT_TRUE(has_content);

    /* PPM for visual inspection without hardware */
    write_ppm("bin/test_metrics.ppm", buf, 320, COMP_METRICS_HEIGHT);

    dashboard_destroy(dash);
}
```

### Regenerating PPM files

Run the test binary directly from the project root (not via ctest):

```sh
bin/test_components
```

If the binary does not exist yet, build first:

```sh
make prepare && make build
```

This overwrites every `bin/test_*.ppm` with fresh renders. Do this after any
visual change to a component — layout tweak, colour adjustment, new component.
`ctest` also regenerates them but the output is less readable for a quick check.

To view a specific component after regenerating:

```sh
open bin/test_alerts.ppm
```

To open all at once:

```sh
open bin/*.ppm
```

To convert one component capture to PNG on macOS:

```sh
sips -s format png bin/test_alerts.ppm --out /tmp/test_alerts.png
```

To batch-convert all component captures on macOS:

```sh
for f in bin/test_*.ppm; do
    sips -s format png "$f" --out "/tmp/$(basename "${f%.ppm}").png"
done
```

### PPM output

`write_ppm` is a local helper in `test_components.c`:

```c
static void write_ppm(const char *path, const uint8_t *buf, int w, int h)
{
    FILE *f = fopen(path, "wb");
    if (!f) return;
    fprintf(f, "P6\n%d %d\n255\n", w, h);
    fwrite(buf, 1, (size_t)(w * h * 3), f);
    fclose(f);
}
```

PPM format: `P6\n<w> <h>\n255\n` + raw RGB888 bytes. Any image viewer opens them.

### ctest working directory

The `test_components` ctest entry has `WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}` set
in `tests/CMakeLists.txt`. This means PPMs are written to `bin/` inside the
project root, which is where `cmake` places compiled binaries. Without this, the
working directory defaults to somewhere inside the `build/` tree, where `bin/`
does not exist and `fopen` silently returns `NULL`.

### Rules

- Import only public headers — never `draw.c` internals.
- Never mock component data. Pass a real populated `comp_*_data_t` through the factory.
- `setUp` must call `xf_set_theme` to guarantee a clean theme for each test.
- One dashboard per test, destroyed at the end.
- Assert buffer non-null AND has non-zero content (transparent = all zeros = nothing rendered).

## Adding a new gfx primitive

A gfx primitive is a reusable drawing helper used by multiple components. It
receives a `ctx` and draws into it directly — no `render` wrapper, no `buf`.

1. Add `src/gfx/<name>.h`:
   ```c
   #ifndef XF_GFX_<NAME>_H
   #define XF_GFX_<NAME>_H
   #include "gfx.h"
   void xf_gfx_<name>(xf_draw_ctx_t *ctx, /* ... */);
   #endif
   ```
2. Add `src/gfx/<name>.c` — include `"gfx/<name>.h"`, implement using `xf_draw_*` API and `xf_gfx_get_theme()` for colours.
3. `CMakeLists.txt` uses `file(GLOB_RECURSE ...)` — no listing update needed.
4. Components include it as `#include "gfx/<name>.h"`.

## Adding a new component

1. Add `comp_<name>.h` — `#include "comp_base.h"`, `COMP_<NAME>_HEIGHT`, data struct, factory declaration.
2. Add `comp_<name>.c` — include `"comp_<name>.h"` and any specific `"gfx/<primitive>.h"` headers needed. `draw` callback reads `xf_get_theme()` and `user_data`, `render` wrapper, factory using local `XF_COMPONENT_DATA` (or `XF_COMPONENT_LIVE` if the component has a `fetch`).
3. `src/components/CMakeLists.txt` uses `file(GLOB_RECURSE COMPONENT_SOURCES "*.c")` — no explicit listing needed.
4. Add a test case to `tests/test_components.c` following the pattern above.
5. Confirm zero compiler warnings with `-Wall -Wextra -Wpedantic`.

## What not to do

- **Do not include `cairo.h` in any file other than `src/draw/draw.c`** — it breaks engine decoupling.
- **Do not write `#include <cairo/cairo.h>` in `draw.c`** — use `#include <cairo.h>` (see above).
- **Do not hard-code colour literals in component source** — use `t->*` fields only.
- **Do not use `return XF_COMPONENT_DATA(...)`** — the macro is a brace-initialiser; wrap it in a local variable.
- **Do not add font loading or font file paths to components** — the theme carries `font_sans` and `font_mono` strings used by `draw.c`; components never interact with fonts directly.
- **Do not call `xf_render` with a NULL function pointer** — even for spacer; pass a no-op draw function instead.
- **Do not access `xf_draw_ctx_t` fields in component code** — it is opaque by design; all drawing goes through the `xf_draw_*` API.

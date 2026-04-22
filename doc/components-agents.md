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

```
src/components/
├── CMakeLists.txt        Static library xf_components; owns Cairo dependency
├── draw.h                Engine-agnostic public draw + theme API
├── draw.c                Cairo implementation — the only file that includes cairo.h
├── comp_header.h/.c      16 px  — date label + status dot
├── comp_outages.h/.c     96 px  — list of active outages with severity rows
├── comp_schedule.h/.c    84 px  — timeline event list
├── comp_metrics.h/.c     38 px  — row of small metric cards
├── comp_oncall.h/.c      32 px  — avatar, name, role, phone
├── comp_deploy.h/.c      24 px  — branch chip, time-ago, outcome label
├── comp_pr_review.h/.c   82 px  — PR list with author avatars
├── comp_sla_gauge.h/.c   76 px  — horizontal progress bars per service
├── comp_sparkline.h/.c   44 px  — filled area chart with last-point dot
├── comp_sprint.h/.c      42 px  — sprint progress bar and labels
├── comp_alerts.h/.c      70 px  — alert list with severity rows
├── comp_error_rate.h/.c  56 px  — bar histogram, colour-coded by severity
├── comp_team_status.h/.c 44 px  — avatar row with online/offline presence
├── comp_checklist.h/.c   84 px  — checklist with checkmarks and strikethroughs
├── comp_build_status.h/.c 34 px — CI build status with outcome pill
├── comp_spacer.h/.c       8 px  — blank vertical gap
└── comp_divider.h/.c      1 px  — thin horizontal rule
```

`xf_components` is a static library. It links Cairo as PUBLIC so that any
target linking `xf_components` automatically pulls in Cairo at link time —
static libraries do not embed their dependencies.

## draw.h — the only header components include for drawing

`draw.h` has zero Cairo types in its public surface. It exposes:

- `xf_rgba_t` — normalized RGBA (double channels in [0.0, 1.0])
- `XF_RGB(0xRRGGBB)` / `XF_RGBA(0xRRGGBB, alpha)` macros — compile-time aggregate initialisers
- `xf_theme_t` — full colour and typography palette
- `xf_theme_default` — default light theme
- `xf_set_theme` / `xf_get_theme` — module-level theme pointer
- `xf_draw_ctx_t` — opaque draw context (components never inspect it)
- Shape and text draw functions: `xf_draw_fill_round_rect`, `xf_draw_circle`, `xf_draw_text`, etc.
- Path API: `xf_draw_begin_path`, `xf_draw_move_to`, `xf_draw_line_to`, `xf_draw_close_path`, `xf_draw_fill`, `xf_draw_stroke`
- `xf_render(buf, w, h, fn, user_data)` — creates a Cairo ARGB32 surface, calls `fn`, converts to RGB888 into `buf`, destroys the surface

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

### Header

```c
#pragma once
#include "../dashboard.h"
#include "draw.h"         /* omit for pure data components (spacer, sprint, etc.) */

#define COMP_FOO_HEIGHT <N>

typedef struct {
    /* All data the component needs — caller fills everything, no defaults */
    char      label[32];
    xf_rgba_t color;      /* caller assigns from theme at creation time */
} comp_foo_data_t;

xf_component_t comp_foo_create(comp_foo_data_t *data);
```

### Implementation

```c
#include "comp_foo.h"
#include "draw.h"

static void draw(xf_draw_ctx_t *ctx, int w, int h, void *user_data)
{
    const xf_theme_t     *t = xf_get_theme();
    const comp_foo_data_t *d = user_data;
    (void)h;  /* suppress warning when height is not used */

    /* all colours from t->*, all content from d->* */
    xf_draw_text(ctx, d->label, 8.0, 12.0, &(xf_text_opts_t){
        .size = 10, .weight = 400, .color = t->text_primary
    });
}

static void render(xf_component_t *self, uint8_t *buf, int w, int h)
{
    xf_render(buf, w, h, draw, self->ctx);
}

xf_component_t comp_foo_create(comp_foo_data_t *data)
{
    /* XF_COMPONENT_DATA expands to a brace-initialiser, not a compound literal.
       A local variable is required; a bare return XF_COMPONENT_DATA(...) is
       invalid C99 and will not compile. */
    xf_component_t c = XF_COMPONENT_DATA(render, data);
    return c;
}
```

For components with no data (spacer, divider) use `XF_COMPONENT(render)` with
the same local-variable pattern:

```c
xf_component_t comp_spacer_create(void)
{
    xf_component_t c = XF_COMPONENT(render);
    return c;
}
```

### NULL in component files

Component `.c` files include `draw.h` which includes `<stddef.h>`, providing
`NULL` for the `XF_COMPONENT*` macros. If you add a component that does not
include `draw.h`, add `#include <stddef.h>` explicitly.

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
to `xf_theme_default` (the call to `xf_get_theme` inside `draw.c` returns the
default if the pointer was never set).

Call it once at startup:

```c
xf_set_theme(&xf_theme_default);
```

To test a custom theme, assign a modified copy and set it:

```c
xf_theme_t dark = xf_theme_default;
dark.text_primary = (xf_rgba_t)XF_RGB(0xEEEEEE);
xf_set_theme(&dark);
/* ... render ... */
xf_set_theme(&xf_theme_default);  /* restore */
```

## Testing

Tests live in `tests/test_components.c`.

### Pattern

```c
void setUp(void) { xf_set_theme(&xf_theme_default); }

static void test_metrics(void)
{
    comp_metrics_data_t d = {0};
    d.count = 2;
    snprintf(d.cards[0].label, sizeof(d.cards[0].label), "UPTIME");
    snprintf(d.cards[0].value, sizeof(d.cards[0].value), "99.94%%");

    xf_component_t comp = comp_metrics_create(&d);

    xf_dashboard_t *dash = dashboard_create(320, COMP_METRICS_HEIGHT);
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

## Adding a new component

1. Add `comp_<name>.h` — `COMP_<NAME>_HEIGHT`, data struct, factory declaration.
2. Add `comp_<name>.c` — `draw` callback reading `xf_get_theme()` and `user_data`, `render` wrapper, factory using local `XF_COMPONENT_DATA`.
3. `src/components/CMakeLists.txt` uses `file(GLOB COMPONENT_SOURCES "*.c")` — no explicit listing needed.
4. Add a test case to `tests/test_components.c` following the pattern above.
5. Confirm zero compiler warnings with `-Wall -Wextra -Wpedantic`.

## What not to do

- **Do not include `cairo.h` in any file other than `draw.c`** — it breaks engine decoupling.
- **Do not write `#include <cairo/cairo.h>` in `draw.c`** — use `#include <cairo.h>` (see above).
- **Do not hard-code colour literals in component source** — use `t->*` fields only.
- **Do not use `return XF_COMPONENT_DATA(...)`** — the macro is a brace-initialiser; wrap it in a local variable.
- **Do not add font loading or font file paths to components** — the theme carries `font_sans` and `font_mono` strings used by `draw.c`; components never interact with fonts directly.
- **Do not call `xf_render` with a NULL function pointer** — even for spacer; pass a no-op draw function instead.
- **Do not access `xf_draw_ctx_t` fields in component code** — it is opaque by design; all drawing goes through the `xf_draw_*` API.

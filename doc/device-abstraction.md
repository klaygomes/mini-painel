# Device Abstraction Refactor

## Problem

`panel.c` (XuanFang protocol) and `turing_panel.c` (Turing protocol) share identical
implementations of several functions. These cannot currently be shared because each file
owns its own `struct xf_device` definition via separate `*_internal.h` headers.

## Identical functions (100% duplicate)

| Function | Lines panel.c | Lines turing_panel.c | Notes |
|----------|--------------|----------------------|-------|
| `effective_width(dev)` | 41–46 | 49–54 | Returns width or height based on orientation |
| `effective_height(dev)` | 48–53 | 56–61 | Same, opposite axis |
| `panel_close(dev)` | 97–102 | 105–110 | `close(fd); free(dev)` |
| `panel_open_auto()` | 87–95 | 95–103 | Calls `port_detect_auto` + `panel_open` |
| `panel_clear(dev)` | 158–179 | 150–171 | Allocs white buffer, calls `panel_display_bitmap` |

## Root cause: struct layout

Both `device/device_internal.h` and `device/turing_device_internal.h` define
`struct xf_device` with the same common fields, but at **different offsets** because the
protocol-specific variant field has a different type/size:

```c
/* device_internal.h (XuanFang) */
struct xf_device {
    int                fd;
    xf_sub_revision_t  sub_revision;   /* XuanFang-specific */
    xf_orientation_t   orientation;
    int                display_width;
    int                display_height;
};

/* turing_device_internal.h (Turing) */
struct xf_device {
    int                fd;
    turing_variant_t   variant;        /* Turing-specific */
    xf_orientation_t   orientation;
    int                display_width;
    int                display_height;
};
```

`orientation`, `display_width`, `display_height` exist in both but at different offsets
because `sub_revision` and `variant` may differ in size. A shared function using these
fields cannot safely cast between the two structs without layout guarantees.

## Proposed design

### 1. Introduce a common base struct

Create `src/device/device_base.h` (internal — included only by device implementation files):

```c
#ifndef DEVICE_BASE_H
#define DEVICE_BASE_H

#include "types.h"

/* Fields shared by all xf_device variants.
 * Must be the FIRST member of every struct xf_device definition so that
 * shared functions can cast xf_device_t* to xf_device_base_t* safely. */
typedef struct {
    int              fd;
    xf_orientation_t orientation;
    int              display_width;
    int              display_height;
} xf_device_base_t;

#endif /* DEVICE_BASE_H */
```

### 2. Update both internal headers to embed base first

```c
/* device_internal.h */
#include "device_base.h"

struct xf_device {
    xf_device_base_t  base;           /* must stay first */
    xf_sub_revision_t sub_revision;
};

/* turing_device_internal.h */
#include "device_base.h"

struct xf_device {
    xf_device_base_t  base;           /* must stay first */
    turing_variant_t  variant;
};
```

**Field access changes** inside `panel.c` and `turing_panel.c` (mechanical, caught by compiler):

| Before | After |
|--------|-------|
| `dev->fd` | `dev->base.fd` |
| `dev->orientation` | `dev->base.orientation` |
| `dev->display_width` | `dev->base.display_width` |
| `dev->display_height` | `dev->base.display_height` |

### 3. Create `src/device/panel_common.c` and `panel_common.h`

`panel_common.h` (private, not installed):

```c
#ifndef PANEL_COMMON_H
#define PANEL_COMMON_H

#include "device_base.h"
#include "panel.h"

int  panel_base_effective_width(const xf_device_base_t *base);
int  panel_base_effective_height(const xf_device_base_t *base);
void panel_close(xf_device_t *dev);
xf_device_t *panel_open_auto(void);

#endif /* PANEL_COMMON_H */
```

`panel_common.c`:

```c
#include "panel_common.h"
#include <unistd.h>
#include <stdlib.h>

int panel_base_effective_width(const xf_device_base_t *base)
{
    return (base->orientation == XF_ORIENT_PORTRAIT ||
            base->orientation == XF_ORIENT_REVERSE_PORTRAIT)
           ? base->display_width : base->display_height;
}

int panel_base_effective_height(const xf_device_base_t *base)
{
    return (base->orientation == XF_ORIENT_PORTRAIT ||
            base->orientation == XF_ORIENT_REVERSE_PORTRAIT)
           ? base->display_height : base->display_width;
}

void panel_close(xf_device_t *dev)
{
    if (!dev) return;
    close(dev->base.fd);
    free(dev);
}

xf_device_t *panel_open_auto(void)
{
    char port[256];
    if (port_detect_auto(port, sizeof(port)) < 0) return NULL;
    return panel_open(port);
}
```

### 4. Handle `panel_clear`

`panel_clear` calls `panel_display_bitmap`, which is protocol-specific. Two options:

**Option A (simplest):** Keep `panel_clear` duplicated in each panel file. It is small (22
lines) and the duplication risk is low since it only calls the public `panel_display_bitmap`.

**Option B:** Move to `panel_common.c` via a function pointer:

```c
void panel_base_clear(xf_device_t *dev,
    int (*display_fn)(xf_device_t*, int, int, int, int, const uint8_t*));
```

Each panel file calls `panel_base_clear(dev, panel_display_bitmap)`. This eliminates the
duplicate but adds one level of indirection.

### 5. Update each panel file

Remove the four functions now provided by `panel_common.c`:
- `effective_width`, `effective_height` — delete; call `panel_base_effective_width/height`
- `panel_close`, `panel_open_auto` — delete; provided by `panel_common.c`

Add `#include "panel_common.h"` to both `panel.c` and `turing_panel.c`.

### 6. Update `src/CMakeLists.txt`

Add `device/panel_common.c` to the `panel` executable source list.

## Impact assessment

| Area | Change |
|------|--------|
| `src/device/device_base.h` | New file |
| `src/device/panel_common.h` | New file |
| `src/device/panel_common.c` | New file |
| `src/device/device_internal.h` | Embed `xf_device_base_t base` as first field |
| `src/device/turing_device_internal.h` | Same |
| `src/device/panel.c` | Field access `dev->x` → `dev->base.x`; remove 4 functions |
| `src/device/turing_panel.c` | Same |
| `src/CMakeLists.txt` | Add `device/panel_common.c` |
| Tests | No changes — public API is unchanged |
| Examples | No changes — public API is unchanged |

## Risk

Low. `struct xf_device` is opaque to all callers outside `src/device/`. The only files that
include `*_internal.h` are `panel.c` and `turing_panel.c`. Field renames are mechanical and
caught immediately by the compiler.

## What stays separate (cannot be shared)

| Function | Reason |
|----------|--------|
| `hello()` | Protocol-specific handshake and response format |
| `panel_set_brightness()` | Different scale (XuanFang linear, Turing inverted) and different protocol calls |
| `panel_set_led()` | XuanFang has RGB LED; Turing has none |
| `panel_set_orientation()` | Different protocol frame formats |
| `panel_display_bitmap()` | Different image conversion (BE vs LE) and protocol calls |
| `panel_is_flagship()` | Reads `sub_revision` vs `variant` — variant-specific |
| `panel_is_brightness_range()` | Same |
| `panel_screen_off/on()` | Different protocol semantics |

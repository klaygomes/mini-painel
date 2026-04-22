# mini-painel

[![CI](https://github.com/klaygomes/mini-painel/actions/workflows/ci.yml/badge.svg)](https://github.com/klaygomes/mini-painel/actions/workflows/ci.yml)

A C99 library for controlling **XuanFang 3.5" USB displays** (Rev B and Flagship).

Plug the display in, open it with one call, and start sending images. The library handles the serial protocol, pixel encoding, and orientation — you work with plain RGB bytes.

---

## Quick start

### What you need

- macOS
- CMake 3.10+
- Cairo (`brew install cairo`)
- A XuanFang 3.5" display connected via USB

### Build

```sh
git clone https://github.com/klaygomes/mini-painel
cd mini-painel
mkdir build && cd build
cmake ..
make
```

The binary lands at `bin/panel`. Plug in the display and run it:

```sh
../bin/panel
```

It auto-detects the device, sets brightness to 80%, and renders a two-row dashboard as a smoke test.

### Run the tests

Tests run without hardware — they use a fake serial backend. Component tests
also write a PPM image per widget to `bin/` so you can visually inspect them.

```sh
cmake .. -DBUILD_TESTING=ON
make
ctest --output-on-failure
```

---

## Using the library in your project

Include `src/panel.h` and `src/types.h`, compile the sources in `src/`, and link them with your program.

```c
#include "panel.h"

int main(void) {
    xf_device_t *dev = panel_open_auto();
    if (!dev) return 1;

    panel_set_brightness(dev, 80);

    /* Send a 320×480 image (your RGB888 bytes) */
    panel_display_bitmap(dev, 0, 0, 320, 480, my_image_data);

    panel_close(dev);
    return 0;
}
```

See [`doc/api.md`](doc/api.md) for the full API reference.

---

## Building a dashboard

The dashboard module lets you compose the screen from independent components arranged in rows. Each component fetches its own data and renders into a plain RGB888 buffer — the library handles layout and blitting.

```c
#include "panel.h"
#include "dashboard.h"

static void render_header(xf_component_t *self, uint8_t *buf, int w, int h) {
    /* fill buf with w*h*3 RGB888 bytes however you like */
}

static void render_stats(xf_component_t *self, uint8_t *buf, int w, int h) {
    /* draw CPU, memory, etc. */
}

int main(void) {
    xf_component_t header = {NULL, render_header, NULL};
    xf_component_t stats  = {NULL, render_stats,  NULL};

    xf_dashboard_t *dash = dashboard_create(320, 480);

    /* Full-width header row, 60 px tall */
    dashboard_add_full_row(dash, &header, 60);

    /* Single stats component filling the remaining 420 px */
    dashboard_add_full_row(dash, &stats, 420);

    xf_device_t *dev = panel_open_auto();
    panel_set_brightness(dev, 80);

    /* Render and send — repeat in a loop for live updates */
    panel_display_bitmap(dev, 0, 0, 320, 480, dashboard_render(dash));

    panel_close(dev);
    dashboard_destroy(dash);
    return 0;
}
```

Components can use any graphics library inside `render()` as long as they write RGB888 into `buf`. See [`doc/dashboard.md`](doc/dashboard.md) for the full dashboard API reference, including multi-column rows, row reordering, and Cairo/stb_image integration.

---

## Supported hardware

| Model | Backplate LED | Full brightness range |
|---|---|---|
| XuanFang Rev B | No | Depends on unit |
| XuanFang Flagship | Yes | Depends on unit |

The library detects capabilities automatically on `panel_open()`.

---

## Project layout

```
src/        Library source and headers
tests/      Unit tests (fake serial, no hardware required)
doc/        API reference
bin/        Compiled binaries (generated)
build/      CMake build directory (generated)
```

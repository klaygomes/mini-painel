# Getting started

## What you need

- macOS
- CMake 3.10+
- Cairo (`brew install cairo`)
- A XuanFang 3.5" display connected via USB

## Build

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

It auto-detects the device, sets brightness to 80%, and renders a demo dashboard to confirm everything is working.

## Run the tests

Tests run without hardware — they use a fake serial backend. Component tests also write a PPM image per widget to `bin/` for visual inspection.

```sh
cmake .. -DBUILD_TESTING=ON
make
ctest --output-on-failure
```

## Using the library in your project

Include `src/panel.h` and `src/types.h`, compile the sources in `src/`, and link with your program.

```c
#include "panel.h"

int main(void) {
    xf_device_t *dev = panel_open_auto();
    if (!dev) return 1;

    panel_set_brightness(dev, 80);
    panel_display_bitmap(dev, 0, 0, 320, 480, my_image_data);

    panel_close(dev);
    return 0;
}
```

`panel_display_bitmap` expects `width × height × 3` bytes of RGB888 data. The library converts to the device's RGB565 format internally.

See [`api.md`](api.md) for the full device API.

## Building a dashboard

The dashboard module lets you compose the screen from independent components arranged in rows. Each component has a `render()` callback that writes pixels into a plain RGB888 buffer — the library handles layout and blitting.

```c
#include "panel.h"
#include "dashboard.h"

static void render_header(xf_component_t *self, uint8_t *buf, int w, int h) {
    /* fill buf with w*h*3 RGB888 bytes */
}

static void render_stats(xf_component_t *self, uint8_t *buf, int w, int h) {
    /* draw CPU, memory, etc. */
}

int main(void) {
    xf_component_t header = XF_COMPONENT(render_header);
    xf_component_t stats  = XF_COMPONENT(render_stats);

    xf_dashboard_t *dash = dashboard_create(320, 480);
    dashboard_add_full_row(dash, &header, 60);
    dashboard_add_full_row(dash, &stats, 420);

    xf_device_t *dev = panel_open_auto();
    panel_set_brightness(dev, 80);

    panel_display_bitmap(dev, 0, 0, 320, 480, dashboard_render(dash));

    panel_close(dev);
    dashboard_destroy(dash);
    return 0;
}
```

See [`dashboard.md`](dashboard.md) for multi-column rows, pagination, live data, and Cairo integration.

## Project layout

```
src/        Library source and headers
tests/      Unit tests (fake serial, no hardware required)
doc/        Documentation
bin/        Compiled binaries (generated)
build/      CMake build directory (generated)
```

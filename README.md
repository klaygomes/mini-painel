# mini-painel

A C99 library for controlling **XuanFang 3.5" USB displays** (Rev B and Flagship).

Plug the display in, open it with one call, and start sending images. The library handles the serial protocol, pixel encoding, and orientation — you work with plain RGB bytes.

---

## Quick start

### What you need

- macOS
- CMake 3.10+
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

It auto-detects the device, sets brightness to 50%, and fills the screen red as a smoke test.

### Run the tests

Tests run without hardware — they use a fake serial backend.

First, install Unity (one time only):

```sh
curl -sSfL https://raw.githubusercontent.com/ThrowTheSwitch/Unity/v2.6.0/src/unity.h \
     -o tests/vendor/unity.h
curl -sSfL https://raw.githubusercontent.com/ThrowTheSwitch/Unity/v2.6.0/src/unity_internals.h \
     -o tests/vendor/unity_internals.h
curl -sSfL https://raw.githubusercontent.com/ThrowTheSwitch/Unity/v2.6.0/src/unity.c \
     -o tests/vendor/unity.c
```

Then build and run:

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

# Agent context

## What this is

A C99 library that implements the serial communication protocol for XuanFang 3.5" USB displays (Rev B and Flagship). macOS only (POSIX termios). No GUI, no system metrics — just device communication.

## Build

Cairo must be installed before the first `cmake` configure:

```sh
brew install cairo        # macOS
sudo apt-get install libcairo2-dev  # Debian/Ubuntu
```

```sh
mkdir -p build && cd build
cmake .. && make                          # main binary
cmake .. -DBUILD_TESTING=ON && make       # with tests
ctest --output-on-failure                 # run tests (from build/)
```

Tests run without hardware. Unity is vendored in `tests/vendor/` — no extra setup needed.
After running `test_components`, visual PPM dumps of every component appear in `bin/`.

## Code conventions

- **Standard**: C99, no extensions beyond POSIX
- **Comments**: why only — never what or how. No decorative separators.
- **Public API**: `src/panel.h` and `src/types.h`. Everything in `src/` that is not one of these two files is internal.
- **Opaque types**: `xf_device_t` is opaque. Its fields live in `src/device_internal.h`, which only `panel.c` and tests include.

## Testing rules

Tests live in `tests/`, use the Unity framework (vendored in `tests/vendor/`), and run without hardware.

### Panel tests (`test_panel.c`)

- Exercise behaviour through the public API only (`panel.h`).
- Never test internal modules (`protocol.c`, `image.c`) directly.
- Never access `xf_device_t` fields — use the public capability functions.
- Use `fake_serial` to simulate the serial port. Call `fake_serial_reset()` in `setUp()` and `fake_serial_clear_writes()` after `panel_open()` so each test starts with an empty write buffer.
- Assert on bytes written to the fake serial port to verify the correct protocol frames are sent. Device-protocol constants (command codes, orientation bytes) are part of the observable hardware contract and are acceptable in test assertions.

### Dashboard tests (`test_dashboard.c`)

- Import `dashboard.h` only — no `panel.h`, no serial, no device dependency.
- Never access `row_t` or `xf_dashboard` internal fields.
- **Pixel-colour pattern**: create small components whose `render()` fills their region with a known solid colour, then read the returned `const uint8_t *` framebuffer at specific `(x, y)` coordinates to verify placement:
  ```c
  /* fb[(y * W + x) * 3 + ch] */
  TEST_ASSERT_EQUAL_UINT8(0xFF, fb[(y * W + x) * 3 + 0]); /* R */
  ```
- **Lifecycle flags**: use static flag variables set inside `fetch()` / `render()` callbacks to verify call order and that `render()` runs even when `fetch()` returns an error or is `NULL`.
- Reset all static flag variables in `setUp()` so tests are isolated.

## Architecture

| File | Responsibility |
|---|---|
| `src/panel.h/.c` | Public API — lifecycle, display control, bitmap |
| `src/serial.h/.c` | POSIX termios serial port (open, read, write, drain) |
| `src/protocol.h/.c` | 10-byte frame builder and sender |
| `src/image.h/.c` | RGB888 → RGB565 BE conversion; 180° pixel rotation |
| `src/port_detect.h/.c` | Auto-detect device via HELLO probe on `/dev/tty.usbmodem*` |
| `src/types.h` | Public types: `xf_orientation_t`, `xf_color_t`, `xf_device_t` (opaque) |
| `src/device_internal.h` | Internal: `xf_sub_revision_t` enum + `struct xf_device` fields |
| `src/dashboard.h/.c` | Row-based immediate-mode layout engine; produces RGB888 framebuffer |
| `src/components/draw.h` | Engine-agnostic draw + theme API (no Cairo types) |
| `src/components/draw.c` | Cairo backend — the only file that includes cairo.h |
| `src/components/comp_*.h/.c` | 17 pre-built Cairo-rendered dashboard widgets |
| `tests/fake_serial.c` | Serial mock for offline testing |
| `tests/test_panel.c` | Unit tests for the panel module |
| `tests/test_dashboard.c` | Unit tests for the dashboard module |
| `tests/test_components.c` | Unit tests for all components; writes PPMs to `bin/` |

See `doc/dashboard-agents.md` for dashboard module internals and testing conventions.
See `doc/components-agents.md` for the component library, draw API, theming, and all Cairo build gotchas.

## Adding a new device revision

1. Create `src/panel_<revision>.h/.c` with its own open/display/command functions.
2. Share `src/serial.h/.c` and `src/protocol.h/.c` if the framing is compatible.
3. Add new capability query functions to its public header; do not extend `panel.h`.
4. Add a new test file in `tests/` using the same fake serial approach.

## What not to do

- Do not add GUI, system metrics, or font rendering — this is a communication library only.
- Do not expose `xf_sub_revision_t` or any struct fields in public headers.
- Do not write tests that import `device_internal.h` or call internal functions.
- Do not add Windows or Linux support without also updating serial.c — the current implementation is macOS-only by design.

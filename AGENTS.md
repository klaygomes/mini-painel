# Agent context

## What this is

A C99 library that implements the serial communication protocol for XuanFang 3.5" USB displays (Rev B and Flagship). macOS only (POSIX termios). No GUI, no system metrics — just device communication.

## Build

```sh
mkdir -p build && cd build
cmake .. && make          # main binary
cmake .. -DBUILD_TESTING=ON && make && ctest  # with tests
```

Tests run without hardware. Unity is vendored in `tests/vendor/` — no extra setup needed.

## Code conventions

- **Standard**: C99, no extensions beyond POSIX
- **Comments**: why only — never what or how. No decorative separators.
- **Public API**: `src/panel.h` and `src/types.h`. Everything in `src/` that is not one of these two files is internal.
- **Opaque types**: `xf_device_t` is opaque. Its fields live in `src/device_internal.h`, which only `panel.c` and tests include.

## Testing rules

- Tests must exercise behavior through the public API only (`panel.h`).
- Never test internal modules (`protocol.c`, `image.c`) directly.
- Never access `xf_device_t` fields from test code — use the public capability functions.
- Device-protocol byte values (command codes, orientation values) are acceptable as expected constants in tests; they are the observable contract between library and hardware, not internal implementation details.

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
| `tests/fake_serial.c` | Serial mock for offline testing |
| `tests/test_panel.c` | Unit tests for the panel module |
| `tests/test_dashboard.c` | Unit tests for the dashboard module |

See `doc/dashboard-agents.md` for dashboard module internals and testing conventions.

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

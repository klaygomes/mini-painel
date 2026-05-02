# Agent context

> **STOP — read `doc/meta-agents.md` before making any edit to this file or any `doc/*-agents.md`.** It defines the rules that govern all agent docs (no restated source values, no hand-written file trees, one fact one home) and lists validation commands to run before committing. Skipping it is the most common cause of doc regressions.

> Single source of truth for every coding agent (Claude Code, Cursor, Aider, Codex, etc.). `CLAUDE.md` is a pointer to this file.

## What this is

A C99 library that drives small USB dashboards. Two protocol families are implemented:

- **Turing Rev A** (`src/protocol/turing_protocol.c`) — the active path used by the demo.
- **XuanFang Rev B / Flagship** (`src/protocol/protocol.c`) — original target, still buildable.

A device abstraction in `src/device/device_base.h` selects the right backend at open time. The serial layer (`src/serial/`) is macOS-only (POSIX termios); the dashboard, draw, theme, and component layers are platform-independent and build anywhere Cairo is available. No GUI, no system metrics — just rendering and device communication.

## Build

Cairo must be installed before the first `cmake` configure:

```sh
brew install cairo                   # macOS
sudo apt-get install libcairo2-dev   # Debian/Ubuntu
```

```sh
mkdir -p build && cd build
cmake .. && make                          # main binary
cmake .. -DBUILD_TESTING=ON && make       # with tests
ctest --output-on-failure                 # run tests (from build/)
```

Tests run without hardware. Unity is vendored in `tests/vendor/` — no extra setup needed. After running `test_components`, visual PPM dumps of every component appear in `bin/`.

## Code conventions

- **Standard**: C99, no extensions beyond POSIX.
- **Public API**: `src/device/panel.h`, `src/types.h`, and `src/json_api/json_api.h`. Everything else under `src/` is internal.
- **Opaque types**: `xf_device_t` is opaque. Its fields live in `src/device/device_internal.h`, included only by `src/device/panel.c` and tests.
- **Single responsibility**: each function performs one conceptual operation.
- **No globals**: functions rely exclusively on their parameters.
- **Error handling**: never ignore return values from standard library functions; check for `NULL`, negative codes, and boundary conditions at call sites. Render-pipeline functions return `xf_result_t` (see `src/status.h`); a non-zero value means the frame is unusable and must not be sent to hardware.
- **Input validation**: validate all external data at the system boundary before processing.
- **Debug logging**: use `DEBUG_LOG(fmt, ...)` from `src/debug.h` for diagnostic traces. The macro is a no-op unless `XF_ENABLE_DEBUG_LOG=1` is defined at compile time (`cmake -DXF_ENABLE_DEBUG_LOG=ON ..`). Never use `printf` for diagnostics; never leave `DEBUG_LOG` calls that reveal sensitive data.
- **Assertions**: use `assert()` to document invariants; never for runtime error handling.
- **Memory ownership**: every `malloc`/`calloc` has a guaranteed `free` path; ownership is explicit at the call site.
- **Bounds checking**: verify buffer sizes before reading or writing.
- **Fixed-width integers**: use `<stdint.h>` types (`uint8_t`, `int32_t`, etc.) over `int`/`long`.
- **Undefined behavior**: avoid all UB patterns (uninitialized reads, signed overflow, null dereference).
- **Compiler flags**: build with `-Wall -Wextra -Werror -pedantic`.
- **Static analysis**: run a static analyser (e.g. `clang --analyze`) to catch memory and UB issues before runtime.

## Comments

**Why-only.** A comment must explain why something non-obvious is done. The code explains what.

Forbidden:
- Section dividers: `/* ── section ── */`, `/* === ... === */`
- Page labels: `/* page 1 */`, `/* page 2 */`
- Narrative "what" comments: `/* draw the circle */`, `/* loop over rows */`
- Block labels that describe structure: `/* mocked data */`, `/* render loop */`, `/* device */`

## Drawing & components

- **No colour literals in component source.** Every colour is a field on the active theme (`t->danger`, `t->text_primary`, etc.). Per-row colours are stored as `xf_rgba_t` fields in the data struct and assigned by the caller from theme values at creation time. Adding a needed shade means extending `xf_theme_t`, not hard-coding hex.
- **Theme colours must be RGB565 round-trip safe.** When adding or changing `XF_RGB`/`XF_RGBA` values in `src/theme/theme_*.c`, quantize RGB channels to nearest RGB565-representable values first. Keep the quantization rule in `doc/theme-tokens.instructions.md` as the single source of truth.
- **`cairo.h` is included only by `src/draw/draw.c`.** The rest of the codebase talks to Cairo through the engine-agnostic `xf_draw_*` API in `draw.h`. Use `#include <cairo.h>` (not `<cairo/cairo.h>`) — `pkg_check_modules` already adds the `cairo/` directory to the include path.
- **`XF_COMPONENT*` macros are brace initialisers.** Always assign to a local variable; `return XF_COMPONENT_DATA(...)` is invalid C99.
- **Heights are derived, not duplicated.** Read `COMP_<NAME>_HEIGHT` from the component header. Do not restate the value in docs or other source files.

## Testing rules

Tests live in `tests/`, use the Unity framework (vendored in `tests/vendor/`), and run without hardware.

PPM debug workflows for rendered output (including `sips` conversion to PNG on macOS) are documented in `doc/components-agents.md` and `doc/daemon-agents.md`.

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

Run `tree src tests -L 2` to see the current layout — do not restate it here, it drifts.

High-level responsibilities by directory:

| Directory | Role |
|---|---|
| `src/device/` | Public panel API (`panel.h`); per-revision backends dispatched via `device_base.h`. Internal headers are off-limits to public callers and tests. |
| `src/serial/` | POSIX termios serial port. macOS-only by design. |
| `src/protocol/` | Wire-frame builders + senders, one family per protocol (`protocol.c` for XuanFang, `turing_protocol.c` for Turing Rev A) plus HELLO-based auto-detection helpers (`port_detect.c`, `turing_port_detect.c`). |
| `src/image/` | RGB888 → RGB565 BE conversion and 180° pixel rotation. |
| `src/dashboard/` | Row-based immediate-mode layout engine. Produces an RGB888 framebuffer; has no graphics dependency and no `panel.h` dependency. |
| `src/draw/` | Engine-agnostic draw API (`draw.h`), Cairo implementation (`draw.c`), layout constants (`layout.h`), page transitions (`transition.h/.c`). |
| `src/theme/` | `xf_rgba_t`, `xf_theme_t`, active-theme singleton, font/weight constants, built-in palettes. |
| `src/components/` | Pre-built dashboard widgets (`comp_*`). Each component header defines `COMP_<NAME>_HEIGHT`. |
| `src/gfx/` | Shared drawing primitives used by components (`gfx/*`). |
| `src/types.h` | Public types: `xf_orientation_t`, `xf_color_t`, `xf_device_t` (opaque). |
| `src/status.h` | `xf_result_t` enum: shared result type for the render pipeline. Read the header for the full error-code catalogue. |
| `src/debug.h` | `DEBUG_LOG` macro: compile-time-gated stderr tracing. Activated by `-DXF_ENABLE_DEBUG_LOG=ON`; zero overhead when off. |
| `src/json_api/` | JSON command API (`json_api.h`). Accepts JSON arrays of ops (`row.*`, `page.*`); returns a JSON reply. Backed by mjson (vendored at `vendor/mjson/`). See `doc/json-api-agents.md`. |
| `src/b64.h/.c` | Base64 encode utility. Used by the daemon's `canvas.get` op. |
| `examples/panel_daemon.c` | WebSocket daemon wrapping `xf_json_api` + panel API. Backed by mongoose (vendored at `vendor/mongoose/`). See `doc/daemon-agents.md`. |
| `tests/` | Unity-based offline tests. `fake_serial.c` mocks the serial port; `test_components.c` writes PPMs to `bin/` for visual review. |

## Adding a new device revision

1. Create `src/device/<revision>_panel.c` and `src/device/<revision>_device_internal.h` (mirror the existing `turing_*` naming).
2. Share `src/serial/serial.h/.c` and `src/protocol/protocol.h/.c` if the framing is compatible; otherwise add a new `src/protocol/<revision>_protocol.h/.c`.
3. Register the backend in `src/device/device_base.h` so `panel_open` can dispatch to it.
4. Add capability query functions to its internal header; do not extend `panel.h`.
5. Add a new test file in `tests/` using the same fake serial approach.

## What not to do

- Do not add GUI, system metrics, or font rendering — this is a communication library only.
- Do not expose `xf_sub_revision_t` or any struct fields in public headers.
- Do not write tests that import `src/device/device_internal.h` or call internal functions.
- Do not add Windows or Linux support without also updating `src/serial/serial.c` — the current implementation is macOS-only by design.

## Editing agent docs

Before editing this file or any `doc/*-agents.md`, read **`doc/meta-agents.md`**. It defines the rules that keep agent docs consistent (no restated source values, no hand-written file trees, one fact one home, etc.) and lists the validation commands to run before committing.

## Reference

- `doc/meta-agents.md` — rules for changing any agent doc. Read first when editing them.
- `doc/agents.md` — device specs, protocol details, layout constants, and page math.
- `doc/components-agents.md` — component module conventions, draw API, theme rules.
- `doc/dashboard-agents.md` — dashboard module internals and testing.
- `doc/glossary.md` — naming conventions and terms.
- `doc/json-api-agents.md` — JSON command API: command shapes, id format, response envelope, error catalog, how to add a kind.
- `doc/daemon-agents.md` — WebSocket daemon: ops, CLI flags, dispatch internals, PPM debug artifact.

## Agent instructions

Read these files before touching the listed paths. They apply to every agent (Claude Code, Cursor, Copilot, Aider, Codex, etc.).

| File | Read when touching |
|------|--------------------|
| `doc/theme-tokens.instructions.md` | `src/theme/**` |
| `doc/component-token-usage.instructions.md` | `src/components/**`, `src/gfx/**`, `src/draw/**` |
</content>
</invoke>
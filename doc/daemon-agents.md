# WebSocket daemon — agent context

> Read this file before touching `examples/panel_daemon.c`, its CMake target, or `src/b64.h/.c`.

## What this is

`examples/panel_daemon.c` is a single-binary WebSocket server that wraps `xf_json_api` and the panel hardware API. Clients connect over WebSocket, send JSON arrays of commands, and receive JSON replies. When a `page.render` op is executed the daemon writes the canvas to a PPM debug file and — if a device is open — pushes the frame to the display via `panel_display_bitmap`.

One device is supported at a time. Multiple WebSocket clients may connect concurrently; the mongoose event loop is single-threaded so commands are serialised automatically.

Mongoose (cesanta/mongoose, MIT) is vendored at `vendor/mongoose/`. The base64 utility used by `canvas.get` lives at `src/b64.h/.c`.

## Commands

All commands follow the same JSON array convention as `doc/json-api-agents.md`. Op strings use dot notation: `<namespace>.<verb>`.

The daemon partitions incoming ops into two sets:

**Layout / render ops** — forwarded to `xf_json_exec` unchanged:

| Op | See |
|----|-----|
| `row.add` | `doc/json-api-agents.md` |
| `row.update` | `doc/json-api-agents.md` |
| `row.remove` | `doc/json-api-agents.md` |
| `row.move_up` / `row.move_down` | `doc/json-api-agents.md` |
| `page.render` | `doc/json-api-agents.md` |

**Daemon ops** — intercepted and handled by the daemon layer before calling `xf_json_exec`:

| Op | Fields | Effect |
|----|--------|--------|
| `device.open` | `"port"` (optional string) | Opens device on `port` or auto-detects. Closes any existing device first. Sets landscape orientation and brightness 80. |
| `device.close` | — | Closes the current device handle. |
| `device.brightness` | `"level"` (integer 0–100) | Sets display brightness. |
| `device.orientation` | `"value"` (string) | Sets orientation. Accepted values: `"landscape"`, `"portrait"`, `"reverse_landscape"`, `"reverse_portrait"`. |
| `canvas.get` | — | Returns the current canvas buffer as a Base64-encoded string. |

Daemon ops and layout ops may be freely mixed in one request array.

## Response envelope

Same format as `doc/json-api-agents.md`. When daemon ops and layout ops are mixed, their results are merged into a single `results` array in request order.

Daemon op success:
```json
{"op":"device.open","ok":true}
```

Daemon op failure:
```json
{"op":"device.open","ok":false,"error":"device not found"}
```

`canvas.get` success:
```json
{"op":"canvas.get","ok":true,"width":480,"height":320,"encoding":"base64","data":"<base64>"}
```

## CLI flags

| Flag | Default | Description |
|------|---------|-------------|
| `-p PORT` | `8765` | TCP port to listen on |
| `-W WIDTH` | `480` | Canvas width in pixels |
| `-H HEIGHT` | `320` | Canvas height in pixels |
| `--padding N` | `0` | Padding passed to `xf_json_create` |
| `--ppm-path PATH` | `panel_daemon_last.ppm` | Path for the debug PPM written after every successful render (relative to working directory) |
| `--no-device` | off | Skip auto-detect; start in headless mode |

## Dispatch internals

`handle_ws_message` uses `mjson_next` to iterate the incoming JSON array in a single pass:
- Daemon ops are identified, executed, and their result objects accumulated.
- All other ops are re-emitted into a filtered array passed to `xf_json_exec`.

After `xf_json_exec` returns, results from both sets are merged into one `{"ok":…,"results":[…]}` envelope. Overall `ok` is `true` only when all daemon ops and the `xf_json_exec` call succeeded.

## PPM debug artifact

After every successful `page.render` the daemon writes the canvas to `--ppm-path` (default: `panel_daemon_last.ppm` in the working directory). This happens whether or not a physical device is connected.

Quick capture workflow:

```sh
# Run the daemon and render from a client, then inspect the latest frame
open panel_daemon_last.ppm
```

Convert the daemon capture to PNG on macOS:

```sh
sips -s format png panel_daemon_last.ppm --out /tmp/panel_daemon_last.png
```

## Base64 module (`src/b64.h/.c`)

Shared C99 utility for Base64 encoding. Public API: see `src/b64.h`. Used only by the `canvas.get` handler. No external dependencies.

## How to add a new daemon op

1. Choose a dot-notation name: `<namespace>.<verb>`.
2. Add a `do_<namespace>_<verb>` handler function in `panel_daemon.c` that writes a result object to its `out` buffer via `snprintf`.
3. Add the op name to `is_daemon_op()`.
4. Add a dispatch case in `dispatch_daemon_op()`.
5. Document the op in the table above.

## Reference

- `doc/json-api-agents.md` — layout/render ops, id format, response envelope, error catalog
- `doc/agents.md` — device specs and panel API constants
- `examples/panel_daemon.c` — implementation
- `src/b64.h` — base64 public API
- `vendor/mongoose/` — vendored mongoose (MIT)

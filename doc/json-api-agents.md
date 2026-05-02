# JSON command API — agent context

> Read this file before touching `src/json_api/`, `tests/test_json_api.c`, or any file under `vendor/mjson/`.

## Overview

`src/json_api/` exposes a single public header (`json_api.h`) that lets a caller control a dashboard entirely through JSON arrays of command objects. The library is pure C99, has no I/O, and adds no new graphics or system dependencies. `mjson` (cesanta/mjson, MIT) is vendored at `vendor/mjson/`.

## Public API

```c
#include "json_api.h"

xf_json_ctx_t *xf_json_create(int width, int height, int pages);
void           xf_json_destroy(xf_json_ctx_t *ctx);
int            xf_json_exec(xf_json_ctx_t *ctx,
                            const char *json, size_t json_len,
                            uint8_t *canvas, size_t canvas_len,
                            char *reply, size_t reply_len);
int            xf_json_page_count(const xf_json_ctx_t *ctx);

/* Canvas size in bytes for a given pixel grid */
#define XF_JSON_CANVAS_SIZE(w, h)  ((size_t)(w) * (size_t)(h) * 3u)
```

- `xf_json_create` returns NULL on allocation failure.
- `xf_json_exec` returns 0 on success, -1 on hard failure (NULL args, allocation failure). The JSON reply always describes per-operation outcomes.
- `canvas` must be at least `XF_JSON_CANVAS_SIZE(width, height)` bytes for render ops; pass NULL/0 to skip rendering.
- `reply` and `reply_len` must always be non-NULL and > 0.

## Op naming convention

Op strings use dot notation: `<namespace>.<verb>`. The `row` namespace operates on dashboard rows; the `page` namespace operates on pages.

## Command shapes

All commands are sent as a **JSON array** of objects. Each object must have `"op"` and (where applicable) `"id"` and `"data"` fields.

### `row.add`

```json
{"op":"row.add","id":"component.<kind>.<name>","data":{...}}
```

Creates a new dashboard row from the given component kind. `id` must be unique in the registry. Appends to the last page.

### `row.update`

```json
{"op":"row.update","id":"component.<kind>.<name>","data":{...}}
```

Patches the existing component's data. Only fields present in `data` are changed. Marks the component dirty.

### `row.remove`

```json
{"op":"row.remove","id":"component.<kind>.<name>"}
```

Removes the row, frees its memory, and unregisters the id.

### `row.move_up` / `row.move_down`

```json
{"op":"row.move_up","id":"component.<kind>.<name>"}
{"op":"row.move_down","id":"component.<kind>.<name>"}
```

Shifts the component one position within its page.

### `row.list`

```json
{"op":"row.list"}
```

Returns the full current state of the registry: every registered row, its kind, page placement, and data payload. No canvas buffer is required.

Response result shape:

```json
{
  "op": "row.list",
  "count": 2,
  "page_count": 1,
  "rows": [
    {
      "id": "component.header.main",
      "kind": "header",
      "page": 0,
      "index": 0,
      "dirty": 0,
      "data": {"date":"Mon","status_text":"ok","status_dot":"#1d9e75ff"}
    },
    ...
  ]
}
```

Fields:

| Field | Type | Meaning |
|---|---|---|
| `count` | int | Number of registered rows |
| `page_count` | int | Total pages the current layout produces |
| `rows[].id` | string | Component id |
| `rows[].kind` | string | Component kind (matches `json_kinds.def`) |
| `rows[].page` | int | 0-based page the row appears on |
| `rows[].index` | int | 0-based position within that page |
| `rows[].dirty` | int | 1 if the component has unsent changes |
| `rows[].data` | object | Current data payload (all fields, colours as `#rrggbbaa`) |

### `page.render`

```json
{"op":"page.render","page":0}
```

Renders page `page` into the supplied canvas buffer. Returns dirty-rect coordinates in the result. `canvas` must be at least `XF_JSON_CANVAS_SIZE(w,h)` bytes.

## ID format

```
component.<kind>.<name>
```

- `<kind>` must match an entry in `json_kinds.def` (e.g. `header`, `deploy`, `sparkline`).
- `<name>` is caller-defined, must be unique per context. Allowed chars: `[A-Za-z0-9_-]`.
- Maximum id length: 63 characters.

## Response envelope

```json
{"ok":true,"results":[...per-op results...]}
```

On any op error:
```json
{"ok":false,"results":[...{"op":"row.add","id":"...","error":"duplicate_id"}...]}
```

If the reply buffer is too small to fit any output:
```json
{"ok":false,"error":"output_truncated"}
```

## Error catalog

| Error string | Meaning |
|---|---|
| `"duplicate_id"` | An `add` was given an id that already exists in the registry |
| `"invalid_id"` | The id does not match the `component.<kind>.<name>` format |
| `"unknown_kind"` | The kind extracted from the id has no entry in `json_kinds.def` |
| `"registry_full"` | The registry has reached `XF_JSON_REG_CAP` (64) entries |
| `"not_found"` | The id was not found for update/remove/move |
| `"canvas_required"` | A render op was issued with NULL canvas |
| `"canvas_too_small"` | The canvas buffer is smaller than `XF_JSON_CANVAS_SIZE(w,h)` |
| `"alloc_failed"` | A memory allocation failed during add |
| `"output_truncated"` | The reply buffer was too small (whole-response fallback) |

## Render result

A successful render op emits:

```json
{"op":"page.render","page":0,"page_count":1,"dirty":[x,y,w,h]}
```

`dirty` is `[0,0,0,0]` when nothing changed since the last render.

## How to add a new kind

1. Add a row to `src/json_api/json_kinds.def`:
   ```c
   XF_KIND(my_kind, sizeof(comp_my_kind_data_t), COMP_MY_KIND_HEIGHT,
           create_my_kind, decode_my_kind, encode_my_kind, 0)
   ```
2. Create `src/json_api/kinds/decode_my_kind.c` implementing `create_my_kind` and `decode_my_kind`, and `src/json_api/kinds/encode_my_kind.c` implementing `encode_my_kind`. Follow the pattern in any existing `decode_*.c` / `encode_*.c`.
3. Add the component implementation to `src/components/` following `doc/components-agents.md`.
4. The CMake glob (`file(GLOB JSON_KIND_SRC CONFIGURE_DEPENDS kinds/*.c)`) picks up the new file automatically.

No changes to `json_api.c`, `json_kinds.c`, or any other file are needed.

## Internal layout

```
src/json_api/
  json_api.h          public header (only new public API)
  json_api.c          dispatcher; implements xf_json_exec
  json_kind.h/.c      kind table (X-macro expansion of json_kinds.def)
  json_kinds.def      X-macro table of all component kinds
  json_registry.h/.c  fixed-capacity id→component registry (cap=64)
  json_decode_util.h/.c  mjson-based field helpers
  json_encode.h/.c    response JSON encoder
  json_encode_util.h  rgba_to_hex helper + includes json_enc_buf.h
  json_enc_buf.h      XF_ENC_APPEND macro (buffer cursor for encode_*.c)
  kinds/              one decode_<kind>.c + encode_<kind>.c per component
vendor/mjson/         vendored cesanta/mjson (MIT)
```

## Reference

- `doc/agents.md` — device, protocol, layout constants
- `doc/components-agents.md` — component conventions
- `doc/dashboard-agents.md` — dashboard internals
- `examples/json_demo.c` — minimal end-to-end usage
- `tests/test_json_api.c` — Unity test suite (15 cases)

# Glossary

Short definitions for terms and naming conventions used across the codebase.

## Prefixes

| Prefix | Meaning |
|---|---|
| `xf_` | Public symbol exported by the library (`xf_device_t`, `xf_set_theme`, `xf_draw_text`). The `xf` stands for the original "XuanFang" target; it now means "this library" regardless of device. |
| `XF_` | Public macros (`XF_RGB`, `XF_COMPONENT_DATA`). |
| `comp_` | Dashboard widget. A component owns a fetch/render pair plus its own data struct. Lives in `src/components/comp_*.{h,c}`. |
| `COMP_` | Component-related macro, almost always `COMP_<NAME>_HEIGHT`. |
| `xf_gfx_` | Drawing primitive shared between components — avatar, pill, chip, dot, etc. Lives in `src/gfx/`. |
| `LAY_` | Row-height or gap constant from `src/draw/layout.h`. |
| `FONT_` / `WEIGHT_` | Font-size or weight constant from `src/theme/theme.h`. |
| `turing_` | Turing Rev A device or protocol implementation. |

## Concepts

- **Component** — a self-contained widget with a `render()` (and optionally `fetch()`) callback. Drawn into a sub-buffer the dashboard owns.
- **Primitive (gfx)** — a reusable drawing helper (no `render`, no `buf`). Takes a `ctx` and draws into it directly. Used by multiple components.
- **Row** — a horizontal slice of the dashboard. A row holds one or more components placed side by side with caller-provided widths.
- **Page** — a vertically scrollable group of rows. The dashboard auto-paginates when the next row would overflow the framebuffer height; `dashboard_render_page(dash, n)` renders page `n`.
- **Sub-buffer** — the per-component RGB888 byte buffer that `dashboard_render` allocates, hands to the component's `render()`, then blits into the framebuffer.
- **Theme** — the active `xf_theme_t` palette and font configuration. Set once at startup with `xf_set_theme`. All component colours are field references on the theme.
- **Backend / device revision** — a panel implementation selected at `panel_open` time via the dispatch table in `src/device/device_base.h`.
- **HELLO probe** — the auto-detection handshake (six identical bytes) used to pick the correct backend for a USB device.

## Files agents commonly need

| File | Why you'd open it |
|---|---|
| `src/draw/layout.h` | Tweak row heights or gaps. |
| `src/theme/theme.h` | Add a colour field, change a font size, change a weight. |
| `src/theme/theme_default.c` | Change the default light palette. |
| `src/components/comp_base.h` | The umbrella include used by every component. |
| `src/gfx/gfx.h` | Base include for primitives; pulls in draw + layout + theme helper. |
| `src/dashboard/dashboard.h` | Public layout/render API. |
| `src/draw/draw.h` | Public draw API; what components actually call. |

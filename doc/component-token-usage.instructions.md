# Theme Token Usage in Components & GFX

> Read this file when writing or editing `src/components/comp_*.c`, `src/components/comp_*.h`, `src/gfx/*.c`, or `src/draw/*.c`.

## Core rule

**Never use a colour literal.** Every colour value passed to a draw call must come from a field on the active theme:

```c
const xf_theme_t *t = xf_get_theme(); /* or xf_gfx_get_theme() inside gfx/ */

/* correct */
xf_draw_fill_round_rect(ctx, x, y, w, h, r, t->danger_bg);

/* forbidden */
xf_draw_fill_round_rect(ctx, x, y, w, h, r, (xf_rgba_t){1.0, 0.2, 0.2, 1.0});
```

## Token selection guide

| Situation | Token |
|-----------|-------|
| Row / card background for an alert state | `{role}_bg` |
| Body text on a tinted background | `{role}_fg` (use `success_fg` etc.) |
| Badge or chip background | `{role}_badge_bg` |
| Badge or chip foreground text | `{role}_badge_fg` |
| Heading or high-contrast value label | `{role}_emphasis` |
| Semi-transparent area under a sparkline | `{role}_subtle` |
| Stroke or text drawn **on top of** a filled circle/rectangle | `on_color` |
| Offline / unknown presence avatar background | `status_offline` |
| Status icon / dot colour | `danger` / `warning` / `success` / `info` directly |
| Progress bar or chart bar with no semantic state | `chart_accent` |
| Orange-tone without error/warning semantics | `caution` |
| De-emphasised supporting text (timestamps, counts) | `text_subtle` |
| Section label / caption | `text_muted` |

## Per-row colours

Components that render variable-state rows must **not** call `xf_get_theme()` inside the row loop to pick colours. Instead:

1. The caller assigns `xf_rgba_t` fields on the row data struct from theme values **at creation time**.
2. The component's `draw()` reads those pre-assigned fields.

```c
/* caller (test or demo) — correct */
d.rows[0].row_bg   = t->danger_bg;
d.rows[0].badge_bg = t->danger_badge_bg;
d.rows[0].badge_fg = t->danger_badge_fg;

/* component draw() — correct */
xf_draw_fill_round_rect(ctx, …, row->row_bg);
```

## RGB565-safe token values

Components, `gfx`, and `draw` code consume theme tokens as authored. They must not introduce per-component colour correction or dithering to compensate for transport quantization.

- If a colour appears unstable on device, fix the token in `src/theme/theme_*.c` by applying the RGB565 round-trip quantization rule from `theme-tokens.instructions.md`.
- Keep component code token-driven: choose the right semantic token, do not post-process channel values in widget code.

## Forbidden patterns

- Any `#define` or `static const xf_rgba_t` with a hardcoded colour.
- Accessing `xf_theme` directly — always go through `xf_get_theme()`.
- Including `<cairo.h>` outside `src/draw/draw.c`.
- Using the old token names (`text_faint`, `text_dimmed`, `white`, `offline`, `*_pill_*`, `*_title_fg`, `info_dark`, `info_fill`, `orange`, `purple_bar`, `deploy_bar`, `deploy_chip_bg`, `deploy_text`, `deploy_text_dark`).

## Adding a colour that does not fit an existing token

1. Read `src/theme/theme.h` and identify the closest role and tier.
2. If genuinely missing, **extend `xf_theme_t`** following the rules in `theme-tokens.instructions.md` — do not invent a local workaround.
3. Add the field to every `theme_*.c` file, not just `theme_default.c`.

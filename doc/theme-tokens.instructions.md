# Theme Token Conventions

> Read this file when editing `src/theme/theme.h`, `src/theme/theme_default.c`, or any `src/theme/theme_*.c`.

## Naming tiers

Every semantic role (`danger`, `warning`, `success`, `info`, `accent`) exposes **at most** these tiers:

| Suffix | Meaning | Example |
|--------|---------|---------|
| *(none)* | Primary indicator — icons, dots, borders | `danger` |
| `_bg` | Subtle tinted background — card, row highlight | `danger_bg` |
| `_fg` | Body text colour on `_bg` | `success_fg` |
| `_badge_bg` | Opaque badge/tag background, denser than `_bg` | `danger_badge_bg` |
| `_badge_fg` | Text inside a badge/tag | `danger_badge_fg` |
| `_emphasis` | Darkest shade — high-contrast headings or values | `danger_emphasis` |
| `_subtle` | Semi-transparent area fill — charts, sparklines | `info_subtle` |

**Never** add tiers that encode a widget name (`_pill_*`, `_chip_*`, `_title_*`, `_bar`) or a raw CSS value (`_dark`, `_fill`).

## Cross-role symmetry

All four main roles must expose **the same set of tiers**. If `danger` gains `_emphasis`, so must `warning`, `success`, and `info`. Asymmetry is only acceptable when no component currently needs the tier for a given role, but the field **must** be added to `xf_theme_t` and `theme_default.c` together.

## Forbidden patterns

- **Raw hue names** (`orange`, `purple`, `green`). Assign a semantic intent: `caution`, `chart_accent`, etc.
- **Widget names** in tokens (`_pill_`, `_chip_`, `_bar`, `_dot`).
- **Layout slot names** (`_title_`, `_label_`, `_header_`).
- **Paint operation names** (`_fill`, `_stroke`).
- **Tonal adjectives** (`_dark`, `_light`, `_faint`, `_dimmed`). Use `_emphasis` for the darkest shade and `_subtle` for transparency.

## Special tokens

| Token | Rule |
|-------|------|
| `on_color` | The only token allowed for foreground drawn **on top of any filled/coloured surface** (checkmark strokes, initials, dot rings). Never use a hardcoded white. |
| `status_offline` | Avatar background for offline/unknown presence. Lives in the surface group. |
| `caution` | Distinct orange tone between `warning` and `danger`. No tiers unless a component needs them. |
| `chart_accent` | Primary bar/progress fill for charts with no semantic state. Add `chart_*` siblings if a second chart colour is needed. |

## Text scale

The scale is ordered **highest to lowest contrast** and must stay monotonically decreasing in luminance when implementing a new theme:

```
text_primary → text_secondary → text_muted → text_subtle
```

Adding a fifth step is allowed; inserting between existing steps is not — rename or remove instead.

## Struct field order

Fields in `xf_theme_t` must appear in this sequence:
1. `background`
2. Text scale (`text_primary` … `text_subtle`)
3. Surfaces (`surface_*`, `on_color`, `status_offline`)
4. Semantic roles in alphabetical order (`danger`, `info`, `success`, `warning`) — each role's tiers grouped together
5. Accent
6. Stand-alone tokens (`caution`, `chart_*`)
7. Domain token groups (`deploy_*`)
8. Typography (`font_sans`, `font_mono`)

## theme_default.c

- Every field in `xf_theme_t` must have a corresponding initialiser.
- Alignment: use spaces (not tabs) to vertically align `=` within each group.
- New raw-hex values must be reviewed for WCAG AA contrast (4.5 : 1) against the background they will appear on.

## RGB565 round-trip safety

The wire format is RGB565, so theme colours must survive an RGB888 -> RGB565 -> RGB888 round-trip without drifting between frames.

- When adding or changing any `XF_RGB(...)` or `XF_RGBA(...)` value in `src/theme/theme_*.c`, quantize the RGB channels to the nearest representable RGB565 value first.
- Quantize each channel independently with nearest rounding, then expand back to 8-bit:
	- `r5 = (r * 31 + 127) / 255`, `r8 = (r5 * 255 + 15) / 31`
	- `g6 = (g * 63 + 127) / 255`, `g8 = (g6 * 255 + 31) / 63`
	- `b5 = (b * 31 + 127) / 255`, `b8 = (b5 * 255 + 15) / 31`
- Do not rely on floor/truncation for palette authoring. Truncation biases colours and is a common source of visible banding and shimmer.
- Alpha in `XF_RGBA` is unchanged by RGB565 transport; quantize only RGB.

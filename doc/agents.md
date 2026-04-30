# Agent Reference: mini-painel Layout & Protocol

This document captures design decisions and calculations established during development so that future sessions can continue without re-deriving them.

---

## Display & Orientation

| Setting | Value |
|---|---|
| Physical display | UsbMonitor 3.5" (VID=0x1A86, PID=0x5722, serial=`USB35INCHIPSV2`) |
| Native resolution | 320 × 480 (portrait) |
| Active orientation | **Landscape** (480 × 320) |
| Active protocol | Turing Rev A (NOT XuanFang) |

In landscape mode the framebuffer is 480 × 320. `panel_set_orientation(dev, XF_ORIENT_LANDSCAPE)` sends the hardware command; `panel_display_bitmap` maps coordinates through `effective_width/effective_height`.

---

## Turing Rev A Protocol

- **HELLO**: send 6 bytes of `0x45`; read 6-byte response.
  - `[0x01 × 6]` → USBMONITOR_3_5 (320 × 480)
  - `[0x02 × 6]` → USBMONITOR_5 (480 × 800)
  - `[0x03 × 6]` → USBMONITOR_7 (600 × 1024)
  - no response → TURING_3_5 (320 × 480, original Turing does not answer)

- **Command frame** (6 bytes, packed coords + cmd byte last):
  ```
  buf[0] = x >> 2
  buf[1] = ((x & 3) << 6) | (y >> 4)
  buf[2] = ((y & 15) << 4) | (ex >> 6)
  buf[3] = ((ex & 63) << 2) | (ey >> 8)
  buf[4] = ey & 0xFF
  buf[5] = cmd
  ```

- **SetOrientation** uses a 16-byte frame: first 6 bytes are the packed command with x=y=ex=ey=0, then:
  - `buf[6]` = orientation + 100 (PORTRAIT=100, REVERSE_PORTRAIT=101, LANDSCAPE=102, REVERSE_LANDSCAPE=103)
  - `buf[7..8]` = effective width (big-endian)
  - `buf[9..10]` = effective height (big-endian)
  - `buf[11..15]` = 0

- **Pixel format**: RGB565 **little-endian** (low byte first). Converting from RGB888:
  ```c
  uint16_t px = (r >> 3) << 11 | (g >> 2) << 5 | (b >> 3);
  out[i*2+0] = px & 0xFF;       // low byte first
  out[i*2+1] = (px >> 8) & 0xFF;
  ```

- **Brightness**: inverted scale — `0` = brightest, `255` = darkest.
  ```c
  val = 255 - (level * 255 / 100);
  turing_proto_send_cmd(fd, TURING_CMD_SET_BRIGHTNESS, val, 0, 0, 0);
  ```

- **Chunk size**: 2560 bytes (320 × 8 rows) avoids USB buffer stalls on macOS.
- **Cooldown**: 50 ms `usleep` after each bitmap prevents corruption on macOS async flush.

---

## Dashboard Padding

`dashboard_create(width, height, padding)` stores the padding. All row management and rendering use `content_width = width - 2*padding` and `content_height = height - 2*padding`.

- `dashboard_add_full_row` automatically uses `content_width`.
- For multi-column rows, use `dashboard_content_width(dash)` instead of a hardcoded display width.
- The framebuffer background and per-component background both come from `xf_get_theme()->background`.

Current demo uses **4 px padding** on all sides.

---

## Layout Constants

Row heights and gaps live in `src/draw/layout.h` (`LAY_*`).
Font sizes and weights live in `src/theme/theme.h` (`FONT_*`, `WEIGHT_*`).
Change values in those headers to rescale all components simultaneously.

### Font Sizes — defined in `src/theme/theme.h`

| Constant | Usage |
|---|---|
| `FONT_XS` | Initials, badges, small indicators |
| `FONT_SM` | Timestamps, secondary labels |
| `FONT_MD` | Body text, section labels |
| `FONT_LG` | Primary content, titles |
| `FONT_XL` | Prominent names |
| `FONT_HERO` | Large metric card values |

### Vertical Dimensions — defined in `src/draw/layout.h`

| Constant | Usage |
|---|---|
| `LAY_HEADER_H` | Section label row height |
| `LAY_TITLE_H` | Inline title bar (sparkline, error rate) |
| `LAY_ROW_SM` | Checklist, schedule rows |
| `LAY_ROW_ALERT` | Alert rows |
| `LAY_ROW_MD` | SLA gauge, PR review rows |
| `LAY_ROW_LG` | Outage rows |
| `LAY_GAP_SM` | Gap between alert rows |
| `LAY_GAP_MD` | Gap between outage rows |
| `LAY_PAD_X` | Universal horizontal padding |

### Component Height Formulas

Row-based heights are computed from layout constants so changing a row constant auto-updates the component height:

```c
COMP_ALERTS_HEIGHT    = LAY_HEADER_H + 3 * LAY_ROW_ALERT + 2 * LAY_GAP_SM
COMP_OUTAGES_HEIGHT   = LAY_HEADER_H + 3 * LAY_ROW_LG    + 2 * LAY_GAP_MD
COMP_SLA_GAUGE_HEIGHT = LAY_HEADER_H + 3 * LAY_ROW_MD
COMP_SCHEDULE_HEIGHT  = LAY_HEADER_H + 4 * LAY_ROW_SM
COMP_PR_REVIEW_HEIGHT = LAY_HEADER_H + 3 * LAY_ROW_MD    + 2 * LAY_GAP_SM
COMP_CHECKLIST_HEIGHT = LAY_HEADER_H + 4 * LAY_ROW_SM
```

Fixed heights live in the relevant component headers. Read the `COMP_<NAME>_HEIGHT`
macro directly instead of restating the numbers here.

Fixed-height components in the demo include `COMP_HEADER_HEIGHT`,
`COMP_DEPLOY_HEIGHT`, `COMP_BUILD_STATUS_HEIGHT`, `COMP_METRICS_HEIGHT`,
`COMP_SPARKLINE_HEIGHT`, `COMP_ERROR_RATE_HEIGHT`, `COMP_SPRINT_HEIGHT`,
`COMP_TEAM_STATUS_HEIGHT`, `COMP_ONCALL_HEIGHT`, `COMP_DIVIDER_HEIGHT`, and
`COMP_SPACER_HEIGHT`.

---

## Demo Page Layout

The demo currently groups rows in this order:

| Page | Components |
|---|---|
| 1 | header + spacer + deploy + div + build + div + metrics + spacer + sparkline + error\_rate |
| 2 | alerts + spacer + outages + spacer + sprint |
| 3 | spacer + team + div + oncall + div + sla + div + schedule |
| 4 | spacer + pr\_review + div + checklist |

A new page starts when the next row would overflow `content_height`. The first row on a page is never overflowed regardless of size.

---

## Theme

The active theme is set once at startup with `xf_set_theme()`. The background field (`t->background`, default `#FFFFFF`) is used for both the framebuffer fill and the per-component Cairo surface background. Components must never hardcode color values — all colors are `t->field` references from the theme.

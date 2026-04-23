# Agent Reference: mini-painel Layout & Protocol

This document captures design decisions and calculations established during development so that future sessions can continue without re-deriving them.

---

## Display & Orientation

| Setting | Value |
|---|---|
| Physical display | UsbMonitor 3.5" (VID=0x1A86, PID=0x5722, serial=`USB35INCHIPSV2`) |
| Native resolution | 320 × 480 px (portrait) |
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

## Layout Constants (`components/layout.h`)

All font sizes and row heights are defined in `layout.h`. Change values there to rescale all components simultaneously.

### Font Sizes (pixels, +2pt from v1)

| Constant | Value | Usage |
|---|---|---|
| `FONT_XS` | 9 | Initials, badges, small indicators |
| `FONT_SM` | 10 | Timestamps, secondary labels |
| `FONT_MD` | 11 | Body text, section labels |
| `FONT_LG` | 12 | Primary content, titles |
| `FONT_XL` | 13 | Prominent names |
| `FONT_HERO` | 14 | Large metric card values |

### Vertical Dimensions

| Constant | Value | Usage |
|---|---|---|
| `LAY_HEADER_H` | 18 | Section label row height |
| `LAY_TITLE_H` | 16 | Inline title bar (sparkline, error rate) |
| `LAY_ROW_SM` | 19 | Checklist, schedule rows |
| `LAY_ROW_ALERT` | 20 | Alert rows |
| `LAY_ROW_MD` | 22 | SLA gauge, PR review rows |
| `LAY_ROW_LG` | 26 | Outage rows |
| `LAY_GAP_SM` | 2 | Gap between alert rows |
| `LAY_GAP_MD` | 4 | Gap between outage rows |
| `LAY_PAD_X` | 8 | Universal horizontal padding |

### Component Height Formulas

Row-based heights are computed from layout constants so changing a row constant auto-updates the component height:

```c
COMP_ALERTS_HEIGHT    = LAY_HEADER_H + 3 * LAY_ROW_ALERT + 2 * LAY_GAP_SM  // = 82
COMP_OUTAGES_HEIGHT   = LAY_HEADER_H + 3 * LAY_ROW_LG    + 2 * LAY_GAP_MD  // = 104
COMP_SLA_GAUGE_HEIGHT = LAY_HEADER_H + 3 * LAY_ROW_MD                        // = 84
COMP_SCHEDULE_HEIGHT  = LAY_HEADER_H + 4 * LAY_ROW_SM                        // = 94
COMP_PR_REVIEW_HEIGHT = LAY_HEADER_H + 3 * LAY_ROW_MD    + 2 * LAY_GAP_SM   // = 90
COMP_CHECKLIST_HEIGHT = LAY_HEADER_H + 4 * LAY_ROW_SM                        // = 94
```

Fixed heights (no clean formula, tuned by inspection):

| Component | Height |
|---|---|
| `COMP_HEADER_HEIGHT` | 20 |
| `COMP_DEPLOY_HEIGHT` | 28 |
| `COMP_BUILD_STATUS_HEIGHT` | 40 |
| `COMP_METRICS_HEIGHT` | 46 |
| `COMP_SPARKLINE_HEIGHT` | 50 |
| `COMP_ERROR_RATE_HEIGHT` | 62 |
| `COMP_SPRINT_HEIGHT` | 48 |
| `COMP_TEAM_STATUS_HEIGHT` | 50 |
| `COMP_ONCALL_HEIGHT` | 40 |
| `COMP_DIVIDER_HEIGHT` | 1 |
| `COMP_SPACER_HEIGHT` | 8 |

---

## Demo Page Layout (landscape 480×320, 4px padding → content 472×312)

| Page | Components | Height |
|---|---|---|
| 1 | header + spacer + deploy + div + build + div + metrics + spacer + sparkline + error\_rate | 264 px |
| 2 | alerts + spacer + outages + spacer + sprint | 250 px |
| 3 | spacer + team + div + oncall + div + sla + div + schedule | 279 px |
| 4 | spacer + pr\_review + div + checklist | 193 px |

A new page starts when the next row would overflow `content_height`. The first row on a page is never overflowed regardless of size.

---

## Theme

The active theme is set once at startup with `xf_set_theme()`. The background field (`t->background`, default `#FFFFFF`) is used for both the framebuffer fill and the per-component Cairo surface background. Components must never hardcode color values — all colors are `t->field` references from the theme.

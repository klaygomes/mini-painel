# Public API Reference

All types and functions are declared in `src/panel.h` and `src/types.h`.

---

## Types

### `xf_device_t`

An opaque handle representing an open device connection. Create one with `panel_open()` or `panel_open_auto()`, and destroy it with `panel_close()`. Never allocate or inspect it directly.

### `xf_orientation_t`

```c
typedef enum {
    XF_ORIENT_PORTRAIT,
    XF_ORIENT_LANDSCAPE,
    XF_ORIENT_REVERSE_PORTRAIT,
    XF_ORIENT_REVERSE_LANDSCAPE
} xf_orientation_t;
```

Portrait and landscape orientations are managed by the display hardware. Reverse variants are handled in software — the library rotates pixel data 180° before sending, so the image appears the right way up.

### `xf_color_t`

```c
typedef struct {
    uint8_t r;
    uint8_t g;
    uint8_t b;
} xf_color_t;
```

An RGB color for the Flagship backplate LED. Values range 0–255 per channel.

---

## Opening and closing a device

### `panel_open`

```c
xf_device_t *panel_open(const char *port);
```

Opens the serial port at `port` (e.g. `"/dev/tty.usbmodemXXX"`), runs the HELLO handshake, and detects the device capabilities.

Returns a device handle on success, `NULL` if the port cannot be opened or the device does not respond correctly.

### `panel_open_auto`

```c
xf_device_t *panel_open_auto(void);
```

Scans `/dev/tty.usbmodem*` and returns the first device that responds to HELLO. Useful when you do not know the port path in advance.

Returns `NULL` if no compatible device is found.

### `panel_close`

```c
void panel_close(xf_device_t *dev);
```

Closes the serial connection and frees the device handle. Safe to call with `NULL`.

---

## Querying capabilities

Capabilities are detected automatically during `panel_open()`.

### `panel_is_flagship`

```c
bool panel_is_flagship(const xf_device_t *dev);
```

Returns `true` for Flagship units, which have a controllable backplate RGB LED. `panel_set_led()` is a no-op on non-flagship units.

### `panel_is_brightness_range`

```c
bool panel_is_brightness_range(const xf_device_t *dev);
```

Returns `true` if the unit supports a continuous brightness range (0–255). Older units only support on/off.

---

## Controlling the display

### `panel_set_orientation`

```c
int panel_set_orientation(xf_device_t *dev, xf_orientation_t orientation);
```

Sets the display orientation. Call this before `panel_display_bitmap()` to ensure coordinates and pixel layout are correct.

Returns 0 on success, -1 on error.

### `panel_set_brightness`

```c
int panel_set_brightness(xf_device_t *dev, int level);
```

Sets the screen brightness. `level` is a percentage in [0, 100].

- Level 0 turns the screen off.
- On range-capable units (see `panel_is_brightness_range`), the value maps linearly to the device range.
- On binary units, any non-zero level means full brightness.

Returns 0 on success, -1 on error.

### `panel_screen_on` / `panel_screen_off`

```c
int panel_screen_on(xf_device_t *dev);
int panel_screen_off(xf_device_t *dev);
```

Convenience wrappers around `panel_set_brightness`. `panel_screen_on` restores brightness to 25%.

### `panel_set_led`

```c
int panel_set_led(xf_device_t *dev, xf_color_t color);
```

Sets the backplate RGB LED color. No-op on non-flagship units (returns 0 without sending anything).

Returns 0 on success, -1 on error.

### `panel_clear`

```c
int panel_clear(xf_device_t *dev);
```

Fills the screen with white. The device has no native clear command, so this sends a full-screen white bitmap internally.

Returns 0 on success, -1 on error.

---

## Displaying images

### `panel_display_bitmap`

```c
int panel_display_bitmap(xf_device_t *dev,
                         int x, int y,
                         int width, int height,
                         const uint8_t *rgb888);
```

Sends a bitmap to the display at position `(x, y)` with dimensions `width × height`.

`rgb888` must point to `width * height * 3` bytes of pixel data in R, G, B order (8 bits per channel). The library converts this to the RGB565 big-endian format the device expects.

Coordinate origin is the top-left corner in the current orientation. The image is clipped to the display bounds automatically.

For `XF_ORIENT_REVERSE_PORTRAIT` and `XF_ORIENT_REVERSE_LANDSCAPE`, coordinates and pixel data are adjusted automatically — pass them as if the display were in the normal portrait or landscape orientation.

Returns 0 on success, -1 on error.

---

## Error handling

All functions return `int` (0 for success, -1 for failure) except the open functions which return `NULL` on failure. Errors are printed to `stderr` with context.

The library does not retry failed serial writes. If the device disconnects mid-session, close and reopen it.

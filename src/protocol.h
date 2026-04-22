#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <stdint.h>
#include <stddef.h>

#define CMD_HELLO           ((uint8_t)0xCA)
#define CMD_SET_ORIENT      ((uint8_t)0xCB)
#define CMD_DISPLAY_BITMAP  ((uint8_t)0xCC)
#define CMD_SET_LIGHTING    ((uint8_t)0xCD)
#define CMD_SET_BRIGHTNESS  ((uint8_t)0xCE)

/* Hardware orientation values for CMD_SET_ORIENT payload[0]. */
#define HW_ORIENT_PORTRAIT  ((uint8_t)0x00)
#define HW_ORIENT_LANDSCAPE ((uint8_t)0x01)

#define DISPLAY_WIDTH       320
#define DISPLAY_HEIGHT      480
#define FRAME_SIZE          10

/* 2560 bytes matches 8 display-width rows; empirically avoids USB buffer stalls. */
#define CHUNK_SIZE          (DISPLAY_WIDTH * 8)

/* 50 ms between bitmaps prevents corruption on macOS where flush is async. */
#define COOLDOWN_US         50000

/* Build and send a 10-byte command frame: [cmd][payload[0..7]][cmd].
 * Pass NULL for payload to send 8 zero bytes.
 * Returns 0 on success, -1 on error. */
int proto_send_cmd(int fd, uint8_t cmd, const uint8_t payload[8]);

/* Send raw bytes (pixel data — not framed).
 * Returns 0 on success, -1 on error. */
int proto_send_raw(int fd, const uint8_t *data, size_t len);

/* Read exactly n bytes. Returns number of bytes actually read. */
int proto_read(int fd, uint8_t *buf, size_t n);

#endif /* PROTOCOL_H */

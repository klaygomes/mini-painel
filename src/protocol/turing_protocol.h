#ifndef TURING_PROTOCOL_H
#define TURING_PROTOCOL_H

#include <stdint.h>
#include <stddef.h>

#define TURING_CMD_RESET          101
#define TURING_CMD_CLEAR          102
#define TURING_CMD_SCREEN_OFF     108
#define TURING_CMD_SCREEN_ON      109
#define TURING_CMD_SET_BRIGHTNESS 110
#define TURING_CMD_SET_ORIENT     121
#define TURING_CMD_DISPLAY_BITMAP 197
#define TURING_CMD_HELLO           69

#define TURING_HELLO_LEN  6
#define TURING_RESP_LEN   6

/* 2560 bytes matches 8 display-width rows; empirically avoids USB buffer stalls. */
#define TURING_CHUNK_SIZE (320 * 8)

/* 50 ms between bitmaps prevents corruption on macOS where flush is async. */
#define TURING_COOLDOWN_US 50000

int turing_proto_send_cmd(int fd, uint8_t cmd, int x, int y, int ex, int ey);
int turing_proto_send_orient(int fd, int orientation, int width, int height);

int turing_proto_send_hello(int fd);
int turing_proto_send_raw(int fd, const uint8_t *data, size_t len);
int turing_proto_read(int fd, uint8_t *buf, size_t n);

#endif /* TURING_PROTOCOL_H */

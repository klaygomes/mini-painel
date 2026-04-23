#include "turing_protocol.h"
#include "serial.h"

#include <string.h>

int turing_proto_send_cmd(int fd, uint8_t cmd, int x, int y, int ex, int ey)
{
    uint8_t buf[6];
    buf[0] = (uint8_t)(x >> 2);
    buf[1] = (uint8_t)(((x & 3) << 6) | (y >> 4));
    buf[2] = (uint8_t)(((y & 15) << 4) | (ex >> 6));
    buf[3] = (uint8_t)(((ex & 63) << 2) | (ey >> 8));
    buf[4] = (uint8_t)(ey & 0xFF);
    buf[5] = cmd;

    int r = serial_write(fd, buf, 6);
    serial_drain(fd);
    return (r < 0) ? -1 : 0;
}

int turing_proto_send_orient(int fd, int orientation, int width, int height)
{
    uint8_t buf[16];
    memset(buf, 0, sizeof(buf));
    buf[5] = TURING_CMD_SET_ORIENT;
    buf[6] = (uint8_t)(orientation + 100);
    buf[7] = (uint8_t)(width >> 8);
    buf[8] = (uint8_t)(width & 0xFF);
    buf[9] = (uint8_t)(height >> 8);
    buf[10] = (uint8_t)(height & 0xFF);

    int r = serial_write(fd, buf, 16);
    serial_drain(fd);
    return (r < 0) ? -1 : 0;
}

int turing_proto_send_hello(int fd)
{
    uint8_t buf[TURING_HELLO_LEN];
    memset(buf, TURING_CMD_HELLO, TURING_HELLO_LEN);
    int r = serial_write(fd, buf, TURING_HELLO_LEN);
    serial_drain(fd);
    return (r < 0) ? -1 : 0;
}

int turing_proto_send_raw(int fd, const uint8_t *data, size_t len)
{
    int r = serial_write(fd, data, len);
    serial_drain(fd);
    return (r < 0) ? -1 : 0;
}

int turing_proto_read(int fd, uint8_t *buf, size_t n)
{
    return serial_read(fd, buf, n);
}

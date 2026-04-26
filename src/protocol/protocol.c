#include "protocol.h"
#include "serial.h"

#include <string.h>

int proto_send_cmd(int fd, uint8_t cmd, const uint8_t payload[8])
{
    uint8_t frame[FRAME_SIZE];

    frame[0] = cmd;
    if (payload != NULL) {
        memcpy(&frame[1], payload, 8);
    } else {
        memset(&frame[1], 0, 8);
    }
    frame[9] = cmd;

    int r = serial_write(fd, frame, FRAME_SIZE);
    serial_drain(fd);
    return (r < 0) ? -1 : 0;
}

int proto_send_raw(int fd, const uint8_t *data, size_t len)
{
    int r = serial_write(fd, data, len);
    serial_drain(fd);
    return (r < 0) ? -1 : 0;
}

int proto_read(int fd, uint8_t *buf, size_t n)
{
    return serial_read(fd, buf, n);
}

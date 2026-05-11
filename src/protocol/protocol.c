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

    int r = serial_write(fd, XF_BYTE_SLICE(frame, FRAME_SIZE));
    serial_drain(fd);
    return (r < 0) ? -1 : 0;
}

int proto_send_raw(int fd, xf_byte_slice_t data)
{
    int r = serial_write(fd, data);
    serial_drain(fd);
    return (r < 0) ? -1 : 0;
}

int proto_read(int fd, xf_byte_buf_t buf)
{
    return serial_read(fd, buf);
}

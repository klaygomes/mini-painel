#include "port_detect.h"
#include "protocol.h"
#include "serial.h"

#include <glob.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

/* Attempt a HELLO handshake on the given port.
 * Returns 0 if the device responds correctly, -1 otherwise. */
static int try_hello(const char *path)
{
    int fd = serial_open(path);
    if (fd < 0) return -1;

    if (serial_configure(fd) < 0) {
        close(fd);
        return -1;
    }

    uint8_t payload[8] = {'H', 'E', 'L', 'L', 'O', 0, 0, 0};
    if (proto_send_cmd(fd, CMD_HELLO, payload) < 0) {
        close(fd);
        return -1;
    }

    uint8_t resp[FRAME_SIZE];
    int n = proto_read(fd, resp, FRAME_SIZE);
    serial_flush_input(fd);
    close(fd);

    if (n != FRAME_SIZE) return -1;
    if (resp[0] != CMD_HELLO || resp[9] != CMD_HELLO) return -1;
    if (resp[1] != 'H' || resp[2] != 'E' || resp[3] != 'L' ||
        resp[4] != 'L' || resp[5] != 'O') return -1;
    if (resp[6] != 0x0A) return -1;

    return 0;
}

int port_detect_auto(char *buf, size_t max_len)
{
    glob_t gl;
    int found = -1;

    if (glob("/dev/tty.usbmodem*", GLOB_NOSORT, NULL, &gl) != 0) {
        return -1;
    }

    size_t i;
    for (i = 0; i < gl.gl_pathc; i++) {
        if (try_hello(gl.gl_pathv[i]) == 0) {
            snprintf(buf, max_len, "%s", gl.gl_pathv[i]);
            found = 0;
            break;
        }
    }

    globfree(&gl);
    return found;
}

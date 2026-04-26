#include "port_detect.h"
#include "turing_protocol.h"
#include "serial.h"

#include <glob.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

/* Returns 0 if the port looks like a Turing device: either it responds to
 * HELLO or it accepts the write without error (TURING_3_5 never responds). */
static int try_hello(const char *path)
{
    int fd = serial_open(path);
    if (fd < 0) return -1;

    if (serial_configure(fd) < 0) {
        close(fd);
        return -1;
    }

    int r = turing_proto_send_hello(fd);

    uint8_t resp[TURING_RESP_LEN];
    turing_proto_read(fd, resp, TURING_RESP_LEN);
    serial_flush_input(fd);
    close(fd);

    /* Any port that accepted the write is a candidate; upstream panel_open
     * will determine the exact variant from the response. */
    return (r < 0) ? -1 : 0;
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

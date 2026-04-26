#ifndef PORT_DETECT_H
#define PORT_DETECT_H

#include <stddef.h>

/* Scan /dev/tty.usbmodem* candidates and return the first one that responds
 * correctly to a HELLO probe.
 * buf:     output buffer for the port path (e.g. "/dev/tty.usbmodemXXX")
 * max_len: size of buf including NUL terminator
 * Returns 0 on success, -1 if no compatible device is found. */
int port_detect_auto(char *buf, size_t max_len);

#endif /* PORT_DETECT_H */

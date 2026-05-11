#ifndef PORT_DETECT_H
#define PORT_DETECT_H

#include "buf.h"

/* Scan /dev/tty.usbmodem* candidates and return the first one that responds
 * correctly to a HELLO probe.
 * buf: output buffer for the port path (e.g. "/dev/tty.usbmodemXXX")
 * Returns 0 on success, -1 if no compatible device is found. */
int port_detect_auto(xf_str_buf_t buf);

#endif /* PORT_DETECT_H */

#ifndef SERIAL_H
#define SERIAL_H

#include <stdint.h>
#include <stddef.h>
#include <termios.h>
#include "buf.h"

#define BAUDRATE B115200

int  serial_open(const char *device_name);
int  serial_configure(int fd);
int  serial_write(int fd, xf_byte_slice_t data);
int  serial_read(int fd, xf_byte_buf_t buf);
void serial_flush_input(int fd);

/* Call after every write on macOS to prevent bitmap corruption. */
void serial_drain(int fd);

#endif /* SERIAL_H */

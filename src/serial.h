#ifndef SERIAL_H
#define SERIAL_H
#include <unistd.h> /* UNIX standard function definitions */
#define BAUDRATE B115200

int serial_open(const char *device_name);
int serial_configure(int fd);
int serial_write(int fd, char *buf, size_t len);
int serial_read(int fd, char *buf, size_t len);
#endif

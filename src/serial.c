#include "serial.h"

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>

int serial_open(const char *device_name)
{
    int fd = open(device_name, O_RDWR | O_NOCTTY | O_NDELAY);
    if (fd < 0) {
        perror("serial_open");
        return -1;
    }
    fcntl(fd, F_SETFL, 0);
    return fd;
}

int serial_configure(int fd)
{
    struct termios options;

    if (tcgetattr(fd, &options) < 0) {
        perror("serial_configure");
        return -1;
    }

    cfsetispeed(&options, BAUDRATE);
    cfsetospeed(&options, BAUDRATE);

    options.c_cflag &= ~PARENB;
    options.c_cflag &= ~CSTOPB;
    options.c_cflag &= ~CSIZE;
    options.c_cflag |= CS8;
    options.c_cflag |= (CLOCAL | CREAD);

    /* Required by the XuanFang device — it will not respond without RTS/CTS. */
    options.c_cflag |= CRTSCTS;

    options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
    options.c_oflag &= ~OPOST;
    options.c_iflag &= ~(IXON | IXOFF | IXANY);

    /* VTIME=10 gives a 1 s timeout so HELLO reads do not block forever. */
    options.c_cc[VMIN]  = 0;
    options.c_cc[VTIME] = 10;

    if (tcsetattr(fd, TCSANOW, &options) < 0) {
        perror("serial_configure");
        return -1;
    }
    return 0;
}

int serial_write(int fd, const uint8_t *buf, size_t len)
{
    ssize_t written = write(fd, buf, len);
    if (written < 0) {
        perror("serial_write");
        return -1;
    }
    return (int)written;
}

int serial_read(int fd, uint8_t *buf, size_t len)
{
    size_t total = 0;
    while (total < len) {
        ssize_t n = read(fd, buf + total, len - total);
        if (n < 0) {
            perror("serial_read");
            break;
        }
        if (n == 0) break;
        total += (size_t)n;
    }
    return (int)total;
}

void serial_flush_input(int fd)
{
    tcflush(fd, TCIFLUSH);
}

void serial_drain(int fd)
{
    tcdrain(fd);
}

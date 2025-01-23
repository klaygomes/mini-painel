#include <errno.h>   /* Error number definitions */
#include <termios.h> /* POSIX terminal control definitions */
#include <unistd.h>  /* UNIX standard function definitions */
#include <fcntl.h>   /* File control definitions */
#include <errno.h>   /* Error number definitions */
#include <stdio.h>   /* Standard input/output definitions */
#include <string.h>  /* String function definitions */

#include "serial.h" /* Serial port definitions */

struct termios options;

int serial_read(int fd, char *ibuf, size_t len)
{
    char *bufptr = ibuf; /* clone the pointer as we intend to use it as a needle */
    int nbytes = 0, tbytes = 0;
    while ((nbytes = read(fd, bufptr, ibuf + sizeof(ibuf) - bufptr - 1)) > 0 && len > tbytes)
    {
        bufptr += nbytes;
        tbytes += nbytes;
    }
    return tbytes;
}

int serial_write(int fd, char *buf, size_t len)
{
    int tries, bytes_written;
    for (tries = 0; tries < 3; tries++)
    {
        bytes_written = write(fd, buf, len);
        if (bytes_written > 0)
            break;
    }
    return bytes_written;
}

int serial_configure(int fd)
{
    tcgetattr(fd, &options);                            /* Get the current options for the port */
    cfsetispeed(&options, BAUDRATE);                    /* Set the input baud rate */
    cfsetospeed(&options, BAUDRATE);                    /* Set the output baud rate */
    options.c_cflag |= (CLOCAL | CREAD);                /* Enable the receiver and set local mode */
    options.c_cflag &= ~PARENB;                         /* No parity */
    options.c_cflag &= ~CSTOPB;                         /* 1 stop bit */
    options.c_cflag &= ~CSIZE;                          /* Mask the character size bits */
    options.c_cflag |= CS8;                             /* Select 8 data bits */
    options.c_cflag &= ~CRTSCTS;                        /* Disable hardware flow control */
    options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG); /* Raw input */
    options.c_oflag &= ~OPOST;                          /* Raw output */
    options.c_cc[VMIN] = 0;                             /* Min characters to read */
    options.c_cc[VTIME] = 1;                            /* Time to wait for data (tenths of seconds) */

    if (tcsetattr(fd, TCSANOW, &options) < 0)
    {
        perror("serial_configure");
        return -1;
    }
    return 0;
}

int serial_open(const char *device_name)
{
    int fd;

    if ((fd = open(device_name, O_RDWR /*read write */ | O_NOCTTY /* no terminal control */ | O_NDELAY /* do not wait */)) == -1)
    {
        perror("serial_open");
    }
    else
    {
        fcntl(fd, F_SETFL, 0);
    }

    return fd;
}

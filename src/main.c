#include <stdlib.h>  /* Standard library definitions */
#include <stdio.h>   /* Standard input/output definitions */
#include <string.h>  /* String function definitions */
#include <unistd.h>  /* UNIX standard function definitions */
#include <fcntl.h>   /* File control definitions */
#include <errno.h>   /* Error number definitions */
#include <termios.h> /* POSIX terminal control definitions */

#define BAUDRATE B115200
#define USB_MODEM_DEVICE "/dev/tty.usbmodemUSB35INCHIPSV21"

struct termios options;

int open_port(void);

int main()
{
    int fd;
    fd = open_port();
    printf("fd = %d\n", fd);
    tcgetattr(fd, &options);

    cfsetispeed(&options, BAUDRATE);
    cfsetospeed(&options, BAUDRATE);

    options.c_cflag |= (CLOCAL | CREAD);                /* Enable the receiver and set local mode */
    options.c_cflag &= ~PARENB;                         /* No parity */
    options.c_cflag &= ~CSTOPB;                         /* 1 stop bit */
    options.c_cflag &= ~CSIZE;                          /* Mask the character size bits */
    options.c_cflag |= CS8;                             /* Select 8 data bits */
    options.c_cflag &= ~CRTSCTS;                        /* Disable hardware flow control */
    options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG); /* Raw input */
    options.c_oflag &= ~OPOST;
    options.c_cc[VMIN] = 0;
    options.c_cc[VTIME] = 1; /* Raw output */

    printf("configuring serial port\n");
    tcsetattr(fd, TCSANOW, &options);
    printf("done\n");

    char buf[6] = {0x45, 0x45, 0x45, 0x45, 0x45, 0x45};

    char buffer[255] = {0};
    char *bufptr;
    int nbytes;
    int tries;

    if ((tries = write(fd, buf, sizeof(buf))) <= 0)
    {
        perror("write");
        return 1;
    }

    bufptr = buffer;
    while ((nbytes = read(fd, bufptr, buffer + sizeof(buffer) - bufptr - 1)) > 0)
    {
        printf("nbytes = %d\n", nbytes);
        bufptr += nbytes;
        if (bufptr[-1] == '\n' || bufptr[-1] == '\r')
            break;
    }

    *bufptr = '\0';

    printf("buffer = %x %x %x %x %x %x \n", buffer[0], buffer[1], buffer[2], buffer[3], buffer[4], buffer[5]);

    close(fd);
    return 0;
}

int open_port(void)
{
    int fd;

    // Open the serial port
    // O_RDWR - Read/Write access to serial port
    // O_NOCTTY - No terminal will control the process
    // O_NDELAY - Non blocking open (does not wait for the device to transmit data)
    if ((fd = open(USB_MODEM_DEVICE, O_RDWR | O_NOCTTY | O_NDELAY)) == -1)
    {
        perror("open_port: Unable to open " USB_MODEM_DEVICE);
    }
    else
    {
        fcntl(fd, F_SETFL, 0);
    }

    return (fd);
}
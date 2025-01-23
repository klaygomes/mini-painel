#include <stdlib.h>  /* Standard library definitions */
#include <stdio.h>   /* Standard input/output definitions */
#include <string.h>  /* String function definitions */
#include <unistd.h>  /* UNIX standard function definitions */
#include <fcntl.h>   /* File control definitions */
#include <errno.h>   /* Error number definitions */
#include <termios.h> /* POSIX terminal control definitions */

#include "serial.h" /* Serial port definitions */
#define USB_MODEM_DEVICE "/dev/tty.usbmodemUSB35INCHIPSV21"

int main()
{
    char buf[6] = {0x45, 0x45, 0x45, 0x45, 0x45, 0x45};
    char buffer[255] = {0};

    int fd = serial_open(USB_MODEM_DEVICE);

    serial_write(fd, buf, sizeof(buf));
    serial_read(fd, buffer, sizeof(buffer));

    printf("buffer = %x %x %x %x %x %x \n", buffer[0], buffer[1], buffer[2], buffer[3], buffer[4], buffer[5]);

    close(fd);
    return 0;
}

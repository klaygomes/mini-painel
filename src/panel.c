#include <inttypes.h> /* Integer types */

struct panel_command
{
    unsigned char id : 2;
    uint8_t x;
    uint8_t y;
    uint8_t length;
    uint8_t width;
} __attribute__((__packed__));

#include "fake_serial.h"
#include "../src/serial/serial.h"

#include <string.h>
#include <stdio.h>

#define FAKE_BUF_SIZE 65536

static uint8_t g_write_buf[FAKE_BUF_SIZE];
static size_t  g_write_len = 0;
static uint8_t g_read_buf[FAKE_BUF_SIZE];
static size_t  g_read_len = 0;

void fake_serial_reset(void)
{
    g_write_len = 0;
    g_read_len  = 0;
    memset(g_write_buf, 0, sizeof(g_write_buf));
    memset(g_read_buf,  0, sizeof(g_read_buf));
}

void fake_serial_clear_writes(void)
{
    g_write_len = 0;
    memset(g_write_buf, 0, sizeof(g_write_buf));
}

void fake_serial_set_response(const uint8_t *data, size_t len)
{
    if (len > FAKE_BUF_SIZE) len = FAKE_BUF_SIZE;
    memcpy(g_read_buf, data, len);
    g_read_len = len;
}

const uint8_t *fake_serial_written_bytes(void) { return g_write_buf; }
size_t         fake_serial_written_len(void)   { return g_write_len; }

int serial_open(const char *device_name)
{
    (void)device_name;
    return 42;
}

int serial_configure(int fd)
{
    (void)fd;
    return 0;
}

int serial_write(int fd, const uint8_t *buf, size_t len)
{
    (void)fd;
    if (g_write_len + len > FAKE_BUF_SIZE) {
        fprintf(stderr, "fake_serial: write buffer overflow\n");
        return -1;
    }
    memcpy(g_write_buf + g_write_len, buf, len);
    g_write_len += len;
    return (int)len;
}

int serial_read(int fd, uint8_t *buf, size_t len)
{
    (void)fd;
    size_t to_copy = (len < g_read_len) ? len : g_read_len;
    memcpy(buf, g_read_buf, to_copy);
    /* Shift remaining bytes forward so subsequent reads get the right data. */
    memmove(g_read_buf, g_read_buf + to_copy, g_read_len - to_copy);
    g_read_len -= to_copy;
    return (int)to_copy;
}

void serial_flush_input(int fd) { (void)fd; }
void serial_drain(int fd)       { (void)fd; }

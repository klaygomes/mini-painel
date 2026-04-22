#ifndef FAKE_SERIAL_H
#define FAKE_SERIAL_H

#include <stdint.h>
#include <stddef.h>

void fake_serial_reset(void);
void fake_serial_clear_writes(void);
void fake_serial_set_response(const uint8_t *data, size_t len);

const uint8_t *fake_serial_written_bytes(void);
size_t         fake_serial_written_len(void);

#endif /* FAKE_SERIAL_H */

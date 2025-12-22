#ifndef SERIAL_H
#define SERIAL_H

#include "types.h"

void serial_init();
void serial_read_char(void);
void serial_write_char(char c);
void serial_write_string(const char* str);
int serial_received(void);
int serial_is_transmit_empty();
int serial_self_test(void);

#endif
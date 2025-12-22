#ifndef KEYBOARD_H
#define KEYBOARD_H

#include "types.h"

#define KEYBOARD_DATA_PORT 0x60
#define KEYBOARD_STATUS_PORT 0x64
#define KEYBOARD_COMMAND_PORT 0x64

char keyboard_getc(void);
void keyboard_clear_buffer(void);
int keyboard_available(void);
char keyboard_getchar(void);

int keyboard_readline(char* buffer, uint32_t size);
const char* keyboard_get_input_line();

void keyboard_init();
void keyboard_interrupt_handler();
// char keyboard_read_scancode();
// char keyboard_scancode_to_ascii(uint8_t scancode);
// void keyboard_handle_input(char c);

void keyboard_self_test(void);

#endif
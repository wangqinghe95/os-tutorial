#ifndef STDIO_H
#define STDIO_H

#include "types.h"
#include "stdarg.h"
#include "vga.h"

typedef enum {
    STDIO_MODE_VGA = 0,
    STDIO_MODE_SERIAL = 1,
    STDIO_MODE_BOTH = 2,
}stdio_mode_t;

void stdio_init(void);
void stdio_set_mode(stdio_mode_t mode);
void stdio_set_color(uint8_t color);

int printf_color(uint8_t color, const char* format, ...);
void printk_color(const char* str, uint8_t color) ;
int putchar(int c);
int puts(const char* str);

int printf(const char* format, ...);
int sprintf(char* buffer, const char* format, ...);
int vsprintf(char* buffer, const char* format, va_list args);
void kprintf(const char* fmt, ...);

void printk(const char* str);
void printk_color(const char* str, uint8_t color);

#endif
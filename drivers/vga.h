#ifndef VGA_H
#define VGA_H

#include "types.h"

/* 屏幕颜色枚举 */
typedef enum {
    BLACK = 0,
    BLUE = 1,
    GREEN = 2,
    CYAN = 3,
    RED = 4,
    MAGENTA = 5,
    BROWN = 6,
    LIGHT_GRAY = 7,
    DARK_GRAY = 8,
    LIGHT_BLUE = 9,
    LIGHT_GREEN = 10,
    LIGHT_CYAN = 11,
    LIGHT_RED = 12,
    LIGHT_MAGENTA = 13,
    YELLOW = 14,
    WHITE = 15
} vga_color;

/* 组合颜色字节 */
static inline uint8_t make_color(vga_color fg, vga_color bg) {
    return fg | (bg << 4);
}

/* 组合字符和颜色 */
static inline uint16_t make_vga_entry(char c, uint8_t color) {
    return (uint16_t)c | ((uint16_t)color << 8);
}

void put_char(char c, uint8_t color);
void set_cursor_pos(uint8_t x, uint8_t y);
void clear_screen(void);

#endif
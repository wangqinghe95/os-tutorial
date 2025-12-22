#include "vga.h"
#include "port.h"
#include "stdarg.h"

#define VIDEO_MEMORY 0xB8000
#define SCREEN_WIDTH 80
#define SCREEN_HEIGHT 25

static uint16_t cursor_x = 0;
static uint16_t cursor_y = 0;

/* 只做VGA相关的函数 */

void clear_screen(void) {
    uint16_t* video_mem = (uint16_t*)VIDEO_MEMORY;
    uint8_t color = make_color(WHITE, BLACK);
    uint16_t blank = make_vga_entry(' ', color);
    
    for (int i = 0; i < SCREEN_WIDTH * SCREEN_HEIGHT; i++) {
        video_mem[i] = blank;
    }
    
    cursor_x = 0;
    cursor_y = 0;
    set_cursor_pos(0, 0);
}

void set_cursor_pos(uint8_t x, uint8_t y) {
    uint16_t pos = y * SCREEN_WIDTH + x;
    
    outb(0x3D4, 0x0F);
    outb(0x3D5, (uint8_t)(pos & 0xFF));
    outb(0x3D4, 0x0E);
    outb(0x3D5, (uint8_t)((pos >> 8) & 0xFF));
    
    cursor_x = x;
    cursor_y = y;
}

static void scroll(void) {
    uint16_t* video_mem = (uint16_t*)VIDEO_MEMORY;
    uint8_t color = make_color(WHITE, BLACK);
    uint16_t blank = make_vga_entry(' ', color);
    
    for (int i = 0; i < (SCREEN_HEIGHT - 1) * SCREEN_WIDTH; i++) {
        video_mem[i] = video_mem[i + SCREEN_WIDTH];
    }
    
    for (int i = (SCREEN_HEIGHT - 1) * SCREEN_WIDTH; i < SCREEN_HEIGHT * SCREEN_WIDTH; i++) {
        video_mem[i] = blank;
    }
    
    cursor_y = SCREEN_HEIGHT - 1;
}

/* 核心：put_char 现在只负责VGA输出 */
void put_char(char c, uint8_t color) {
    uint16_t* video_mem = (uint16_t*)VIDEO_MEMORY;
    
    if (c == '\n') {
        cursor_x = 0;
        cursor_y++;
    } 
    else if (c == '\r') {
        cursor_x = 0;
    }
    else if (c == '\t') {
        cursor_x = (cursor_x + 4) & ~(4 - 1);
    }
    else {
        uint16_t index = cursor_y * SCREEN_WIDTH + cursor_x;
        video_mem[index] = make_vga_entry(c, color);
        cursor_x++;
    }
    
    if (cursor_x >= SCREEN_WIDTH) {
        cursor_x = 0;
        cursor_y++;
    }
    
    if (cursor_y >= SCREEN_HEIGHT) {
        scroll();
    }
    
    set_cursor_pos(cursor_x, cursor_y);
}

/* 移除旧的双输出函数 */
// void printk() 和 printk_color() 已经移到 stdio.c
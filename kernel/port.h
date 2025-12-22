#ifndef PORT_H
#define PORT_H

#include "types.h"

/* 端口输出函数 */
static inline void outb(uint16_t port, uint8_t value) {
    asm volatile ("outb %0, %1" : : "a"(value), "Nd"(port));
}

static inline uint8_t inb(uint16_t port) {
    uint8_t result;
    asm volatile ("inb %1, %0" : "=a"(result) : "Nd"(port));
    return result;
}

static inline void io_delay(void)
{
    for(int i = 0; i < 4; i++) {
        __asm__ volatile("nop");
    }
}

#endif
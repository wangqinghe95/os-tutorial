#include "serial.h"
#include "port.h"

#define COM1_PORT 0x3F8

void serial_init()
{
    outb(COM1_PORT + 1, 0x00);
    outb(COM1_PORT + 3, 0x80);
    outb(COM1_PORT + 0, 0x0C);
    outb(COM1_PORT + 1, 0x00);
    outb(COM1_PORT + 3, 0x03);
    outb(COM1_PORT + 2, 0xC7);
    outb(COM1_PORT + 4, 0x0B);
}

void serial_read_char(void)
{
    while (!serial_received());

    inb(COM1_PORT);
}

int serial_is_transmit_empty()
{
    return inb(COM1_PORT + 5) & 0x20;
}


void serial_write_char(char c)
{
    // 特殊字符处理
    if (c == '\n') {
        // 在串口中，换行需要加上回车
        while (!serial_is_transmit_empty());
        outb(COM1_PORT, '\r');
    }

    while (!serial_is_transmit_empty());

    outb(COM1_PORT, c);
    
}

void serial_write_string(const char* str)
{
    while (*str)
    {
        serial_write_char(*str++);
    }
    
}

int serial_received(void)
{
    return inb(COM1_PORT + 5) & 0x01;
}

/* 串口自检 */
int serial_self_test(void) {
    // 简单的回环测试
    outb(COM1_PORT + 4, 0x1E);  // 启用回环模式
    
    // 发送测试字符
    outb(COM1_PORT, 0x55);
    
    // 检查是否收到
    if (inb(COM1_PORT) != 0x55) {
        outb(COM1_PORT + 4, 0x0B);  // 禁用回环模式
        return 0;  // 测试失败
    }
    
    outb(COM1_PORT + 4, 0x0B);  // 禁用回环模式
    return 1;  // 测试成功
}

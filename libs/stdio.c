#include "stdio.h"
#include "vga.h"
#include "serial.h"
#include "port.h"
#include "string.h"

#ifdef ENABLE_SERIAL
    #define SERIAL_ENABLED 1
#else
    #define SERIAL_ENABLED 0
#endif

static stdio_mode_t current_mode = STDIO_MODE_VGA;
uint8_t current_color;

typedef enum {
    FLAG_NONE = 0,
    FLAG_ZERO = 1 << 0,
    FLAG_LEFT = 1 << 1,
} format_flags;

void stdio_init(void)
{
    current_color = make_color(WHITE, BLACK);
    #if SERIAL_ENABLED
    current_mode = STDIO_MODE_BOTH;
    serial_init();
    #endif

}

void stdio_set_mode(stdio_mode_t mode)
{
    #if !SERIAL_ENABLED
    if (mode == STDIO_MODE_SERIAL || mode == STDIO_MODE_BOTH) {
        return; // 串口未启用，忽略
    }
    #endif
    current_mode = mode;   
}

/* 设置输出颜色 */
void stdio_set_color(uint8_t color) 
{
    current_color = color;
}

/* 核心输出函数：统一处理VGA和串口输出 */
static void stdio_output_char(char c) 
{
    if (current_mode == STDIO_MODE_VGA || current_mode == STDIO_MODE_BOTH) {
        put_char(c, current_color);
    }
    
    #if SERIAL_ENABLED
    if (current_mode == STDIO_MODE_SERIAL || current_mode == STDIO_MODE_BOTH) {
        serial_write_char(c);
    }
    #endif
}

static void stdio_output_string(const char* str) {
    while (*str) {
        stdio_output_char(*str++);
    }
}

int printf(const char* format, ...)
{
    char buffer[256];
    va_list args;

    va_start(args, format);
    int len = vsprintf(buffer, format, args);
    va_end(args);

    stdio_output_string(buffer);

    return len;
}

int putchar(int c)
{
    stdio_output_char((char)c);
    return c;
}

int puts(const char* str)
{
    stdio_output_string(str);
    stdio_output_char('\n');
    return 0;
}

int sprintf(char* buffer, const char* format, ...)
{
    va_list args;
    va_start(args, format);
    int result = vsprintf(buffer, format, args);
    va_end(args);
    return result;
}

bool isdigit(char c)
{
    return (c >= '0' && c <= '9');
}

static void format_string(char* dest, const char* src, int width, format_flags flags) {
    int len = strlen(src);
    int pad = width > len ? width - len : 0;
    
    if (flags & FLAG_LEFT) {
        strcpy(dest, src);
        memset(dest + len, ' ', pad);
    } else {
        memset(dest, (flags & FLAG_ZERO) ? '0' : ' ', pad);
        strcpy(dest + pad, src);
    }
    dest[width > len ? width : len] = '\0';
}

static void format_number(char* dest, int num, int base, int width, format_flags flags) {
    char num_buf[32];
    itoa(num, num_buf, base);
    format_string(dest, num_buf, width, flags);
}

int printf_color(uint8_t color, const char* format, ...) 
{
    uint8_t old_color = current_color;
    stdio_set_color(color);
    
    char buffer[256];
    va_list args;
    va_start(args, format);
    int len = vsprintf(buffer, format, args);
    va_end(args);
    
    stdio_output_string(buffer);
    stdio_set_color(old_color);
    return len;
}

/* 输出字符串（指定颜色） */
void printk_color(const char* str, uint8_t color)
{
    uint8_t old_color = current_color;
    stdio_set_color(color);
    stdio_output_string(str);
    stdio_set_color(old_color);
}

void printk(const char* str) {
    stdio_output_string(str);
}

int vsprintf(char* buffer, const char* format, va_list args)
{
    char* ptr = buffer;
    char num_buffer[32];

    while (*format)
    {
        if(*format == '%') {
            format++;

            format_flags flags = FLAG_NONE;
            while (*format == '0' || *format == '-')
            {
                if(*format == '0') flags |= FLAG_ZERO;
                if(*format == '-') flags |= FLAG_LEFT;
                format++;
            }

            int width = 0;
            while (isdigit(*format))
            {
                width = width * 10 + (*format - '0');
                format++;
            }
            
            switch (*format)
            {
            case 'd':
            case 'i':{
                int num = va_arg(args, int);
                format_number(num_buffer, num, 10, width, flags);
                // itoa(int_arg, num_buffer, 10);
                strcpy(ptr, num_buffer);
                ptr += strlen(num_buffer);
                break;
            }

            case 'u':{
                unsigned int num = va_arg(args, unsigned int);
                format_number(num_buffer, num, 10, width, flags);
                strcpy(ptr, num_buffer);
                ptr += strlen(num_buffer);
                break;
            }
            case 'x':
            case 'X':{
                unsigned int num = va_arg(args, unsigned int);
                format_number(num_buffer, num, 16, width, flags);
                strcpy(ptr, num_buffer);
                ptr += strlen(num_buffer);
                break;
            }
            case 'c':
                *ptr++ = (char)va_arg(args, int);
                break;

            case 's':
                char* str = va_arg(args, char*);
                format_string(num_buffer, str ? str : "(null)", width, flags);
                strcpy(ptr, num_buffer);
                ptr += strlen(num_buffer);
                break;

            case '%':
                *ptr++ = '%';
                break;

            default:
                *ptr++ = '%';
                *ptr++ = *format;
                break;
            }
        }
        else {
            *ptr++ = *format;
        }
        format++;
    }

    *ptr = '\0';
    return ptr - buffer;
}


void kprintf(const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    
    char buffer[256];
    int len = vsprintf(buffer, fmt, args);
    
    // 输出到串口（用于终端显示）
    for (int i = 0; i < len; i++) {
        putchar(buffer[i]);
    }
    
    // 输出到 VGA（屏幕显示）
    printk(buffer);
    
    va_end(args);
}

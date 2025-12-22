#include "string.h"



void memset(void* ptr, uint8_t value, uint32_t size)
{
    uint8_t* p = (uint8_t*)ptr;
    for(uint32_t i = 0; i < size; i++)
    {
        p[i] = value;
    }
}


void itoa(int value, char* str, int base)
{
    char* ptr = str;
    char* ptr1 = str;

    char tmp_char;
    int tmp_value;

    if(0 == value) {
        *ptr++ = '\0';
        *ptr = '\0';
        return;
    }

    while (value)
    {
        tmp_value = value;
        value /= base;
        *ptr++ = "zyxwvutsrqponmlkjihgfedcba9876543210123456789abcdefghijklmnopqrstuvwxyz"[35 + (tmp_value - value * base)];
            
    }

    *ptr-- = '\0';

    while (ptr1 < ptr)
    {
        tmp_char = *ptr;
        *ptr-- = *ptr1;
        *ptr1++ = tmp_char;
    }
}


char* strcpy(char* dest, const char* src)
{
    char* ptr = dest;
    while (*src)
    {
        *ptr++ = *src++;
    }

    *ptr = '\0';

    return dest;    
}

size_t strlen(const char* str)
{
    size_t len = 0;
    while (str[len])
    {
        len++;
    }

    return len;    
}

int strcmp(const char* s1, const char* s2)
{
    while (*s1 &&(*s1 == *s2))
    {
        s1++;
        s2++;
    }

    return *(const unsigned char*)s1 - *(const unsigned char*)s2;
}


#include <stddef.h> // 用于 size_t 类型

int strncmp(const char *s1, const char *s2, size_t n) {
    if (n == 0) {
        return 0;
    }
    
    while (n-- > 0 && *s1 && *s2) {
        if (*s1 != *s2) {
            return (*(unsigned char *)s1 - *(unsigned char *)s2);
        }
        s1++;
        s2++;
    }
    
    if (n == (size_t)-1 || *s1 || *s2) {
        return (*(unsigned char *)s1 - *(unsigned char *)s2);
    }
    
    return 0;
}
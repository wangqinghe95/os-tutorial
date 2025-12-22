#ifndef STRING_H
#define STRING_H

#include "types.h"

void memset(void* ptr, uint8_t value, uint32_t size);
void itoa(int value, char* str, int base);
char* strcpy(char* dest, const char* src);
size_t strlen(const char* str);

int strcmp(const char* s1, const char* s2);

#endif
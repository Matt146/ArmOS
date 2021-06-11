#ifndef UTIL_H
#define UTIL_H

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

static char __text_buff_unsigned_long_to_str[25];
char* unsigned_long_to_str(unsigned long zahl);
void memsetb(uint8_t* src, uint8_t data, size_t count);
int strncmp(const char* str1, const char* str2, size_t n);

#endif // UTIL_H
#ifndef UTIL_H
#define UTIL_H

#include <stdint.h>
#include <stddef.h>
#include "../sched/mutex.h"

#define RAND_MAX 32767
#define IMAX_BITS(m) ((m)/((m)%255+1) / 255%255*8 + 7-86/((m)%255+12))
#define RAND_MAX_WIDTH IMAX_BITS(RAND_MAX)
_Static_assert((RAND_MAX & (RAND_MAX + 1u)) == 0, "RAND_MAX not a Mersenne number");

static char __text_buff_unsigned_long_to_str[25];
char* unsigned_long_to_str(unsigned long zahl);
char *int_base_to_string(size_t number, size_t base);
void memsetb(uint8_t* src, uint8_t data, size_t count);
void* memcpy(void* dest, const void* src, size_t len);
size_t strlen(const char *str);

int rand(void);
void srand(unsigned int seed);

uint64_t rand64(void);
uint64_t pow(uint64_t x, uint64_t n);   // returns x^n

#endif // UTIL_H

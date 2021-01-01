#include "bits.h"

// create_bitmask - Creates a bitmask to extract bits a...b
// from an integer. All you need to do to use it is get the value
// and AND it with the other number.
// IE:
// uint64_t mask = create_bitmask(12, 16);
// uint64_t result = mask & NUMBER_YOU_WANT_TO_EXTRACT_BITS_FROM;
uint64_t create_bitmask(uint64_t a, uint64_t b) {
    uint64_t r = 0;
    for (uint64_t i = a; i <= b; i++) {
        r |= (1 << i);
    }

    return r;
}

/**
 * C++ version 0.4 char* style "itoa":
 * Written by Lukás Chmela
 * Released under GPLv3.
 */
 char* itoa(int value, char* result, int base) {
    // check that the base if valid
    if (base < 2 || base > 36) { *result = '\0'; return result; }

    char* ptr = result, *ptr1 = result, tmp_char;
    int tmp_value;

    do {
        tmp_value = value;
        value /= base;
        *ptr++ = "zyxwvutsrqponmlkjihgfedcba9876543210123456789abcdefghijklmnopqrstuvwxyz" [35 + (tmp_value - value * base)];
    } while ( value );

    // Apply negative sign
    if (tmp_value < 0) *ptr++ = '-';
    *ptr-- = '\0';
    while(ptr1 < ptr) {
        tmp_char = *ptr;
        *ptr--= *ptr1;
        *ptr1++ = tmp_char;
    }
    return result;
}
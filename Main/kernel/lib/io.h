#ifndef IO_H
#define IO_H

#include <stdint.h>
#include <stddef.h>

static inline void out_b(uint16_t port, uint8_t val) {
    asm volatile ( "outb %0, %1" : : "a"(val), "Nd"(port) );
}

static inline void out_w(uint16_t port, uint16_t val) {
    asm volatile ( "outw %0, %1" : : "a"(val), "Nd"(port) );
}

static inline void out_d(uint16_t port, uint32_t val) {
    asm volatile ( "outl %0, %1" : : "a"(val), "Nd"(port) );
}

static inline uint8_t in_b(uint16_t port) {
    uint8_t ret;
    asm volatile ( "inb %1, %0"
                   : "=a"(ret)
                   : "Nd"(port) );
    return ret;
}

static inline uint16_t in_w(uint16_t port) {
    uint16_t ret;
    asm volatile ( "inw %1, %0"
                   : "=a"(ret)
                   : "Nd"(port) );
    return ret;
}

static inline uint32_t in_d(uint16_t port) {
    uint32_t ret;
    asm volatile ( "inl %1, %0"
                   : "=a"(ret)
                   : "Nd"(port) );
    return ret;
}

inline uint64_t rdmsr(uint64_t msr) {
    uint64_t rax, rdx;
    asm volatile ("rdmsr" : "=a"(rax), "=d"(rdx) : "c"(msr));
    return (rdx << 32) | rax;
}

inline void wrmsr(uint64_t msr, uint64_t data) {
    uint64_t rax = (uint32_t)data;
    uint64_t rdx = data >> 32;
    asm volatile ("wrmsr" :: "a"(rax), "d"(rdx), "c"(msr));
}

// puts a DEBUG value into rax
// DEBUG: 0xDEADBEEF
extern void DEBUG();

#endif // IO_H
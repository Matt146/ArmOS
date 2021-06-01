#ifndef IO_H
#define IO_H

#include <stdint.h>
#include <stddef.h>

extern void out_b(uint16_t port, uint8_t data);
extern uint8_t in_b(uint16_t port);

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
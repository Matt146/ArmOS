#ifndef IO_H
#define IO_H

#include <stdint.h>
#include <stddef.h>

extern void out_b(uint16_t port, uint8_t data);
extern uint8_t in_b(uint16_t port);

// puts a DEBUG value into rax
// DEBUG: 0xDEADBEEF
extern void DEBUG();

#endif // IO_H
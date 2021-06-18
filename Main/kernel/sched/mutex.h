#ifndef MUTEX_H
#define MUTEX_H

#include <stdint.h>
#include <stddef.h>

extern void mutex_lock(uint8_t* mutex);
extern void mutex_unlock(uint8_t* mutex);

#endif // MUTEX_H
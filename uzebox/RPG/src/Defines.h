#ifndef DEFINES_H_
#define DEFINES_H_

#include <stdint.h>

#if defined(_WIN32) || defined(_WIN64)
#define false   0
#define true    1
typedef uint8_t bool;
#else
#include <stdbool.h>
#endif

#define BIT(x) ((0x1) << (x))

#define INVALID_ID -1

#endif // DEFINES_H_
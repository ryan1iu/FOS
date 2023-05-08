#ifndef INC_ASSERT_H
#define INC_ASSERT_H

#include <inc/stdio.h>

void _panic(const char*, int, const char*, ...);
#define panic(...) _panic(__FILE__, __LINE__, __VA_ARGS__)

#define assert(x)                                    \
    do {                                             \
        if (!(x)) panic("assertion failed: %s", #x); \
    } while (0)

#endif

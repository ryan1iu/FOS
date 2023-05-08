#include <inc/assert.h>

#include "inc/monitor.h"

void _panic(const char *file, int line, const char *fmt, ...) {
    va_list ap;

    // Be extra sure that the machine is in as reasonable state
    asm volatile("cli; cld");

    va_start(ap, fmt);
    cprintf("kernel panic at %s:%d: ", file, line);
    vcprintf(fmt, ap);
    cprintf("\n");
    va_end(ap);
    /* break into the kernel monitor */
    while (1) readline();
}

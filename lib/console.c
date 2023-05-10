#include <lib/lib.h>

void putchar(int ch) {
    char c = ch;
    sys_puts(&c, 1);
}

int getchar(void) {
    int r;
    while ((r = sys_getc()) == 0) sys_yield();
    return r;
}

#include <inc/console.h>
#include <inc/stdio.h>

static void putch(int ch, int *cnt) {
    console_putc(ch, COLOR_LIGHT_WHITE);
    *cnt++;
}

int vcprintf(const char *fmt, va_list ap) {
    int cnt = 0;

    vprintfmt((void *)putch, &cnt, fmt, ap);
    return cnt;
}

void cprintf(const char *fmt, ...) {
    va_list ap;
    int cnt;

    va_start(ap, fmt);
    cnt = vcprintf(fmt, ap);
    va_end(ap);
}

#include <inc/stdio.h>
#include <lib/lib.h>

struct printbuf {
    int idx;  // current buffer index
    int cnt;  // total bytes printed so far
    char buf[256];
};

static void putch(int ch, struct printbuf *b) {
    b->buf[b->idx++] = ch;
    if (b->idx == 256 - 1) {
        sys_puts(b->buf, b->idx);
        b->idx = 0;
    }
    b->cnt++;
}

int vcprintf(const char *fmt, va_list ap) {
    struct printbuf b;

    b.idx = 0;
    b.cnt = 0;
    vprintfmt((void *)putch, &b, fmt, ap);
    sys_puts(b.buf, b.idx);

    return b.cnt;
}

void cprintf(const char *fmt, ...) {
    va_list ap;
    int cnt;

    va_start(ap, fmt);
    cnt = vcprintf(fmt, ap);
    va_end(ap);
}

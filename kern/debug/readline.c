#include <inc/stdio.h>
#include <inc/types.h>

#include "inc/console.h"

#define BUFLEN 1024
static char buf[BUFLEN];

char *readline(const char *prompt) {
    int i, c;

    console_putc('>', COLOR_LIGHT_GREEN);
    console_putc('>', COLOR_LIGHT_FUCHSINE);
    console_putc('>', COLOR_LIGHT_BLUE);
    if (prompt != NULL) cprintf("%s", prompt);

    i = 0;
    while (1) {
        c = console_getc();
        if (c < 0) {
            cprintf("read error: %e\n", c);
            return NULL;
        } else if ((c == '\b' || c == '\x7f') && i > 0) {
            console_putc('\b', COLOR_LIGHT_WHITE);
            i--;
        } else if (c >= ' ' && i < BUFLEN - 1) {
            console_putc(c, COLOR_LIGHT_WHITE);

            buf[i++] = c;
        } else if (c == '\n' || c == '\r') {
            console_putc('\n', COLOR_LIGHT_WHITE);

            buf[i] = 0;
            return buf;
        }
    }
}

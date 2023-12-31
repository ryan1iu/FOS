#include <lib/lib.h>

#define BUFLEN 1024
static char buf[BUFLEN];

char *readline(const char *prompt) {
    int i, c;

    if (prompt != NULL) cprintf("%s", prompt);

    i = 0;
    while (1) {
        c = getchar();
        if (c < 0) {
            cprintf("read error: %e\n", c);
            return NULL;
        } else if ((c == '\b' || c == '\x7f') && i > 0) {
            putchar('\b');
            i--;
        } else if (c >= ' ' && i < BUFLEN - 1) {
            putchar(c);
            buf[i++] = c;
        } else if (c == '\n' || c == '\r') {
            putchar('\n');
            buf[i] = 0;
            return buf;
        }
    }
}

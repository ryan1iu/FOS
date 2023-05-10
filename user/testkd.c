#include <lib/lib.h>

void umain(int argc, char **argv) {
    for (int i = 0; i < 2; i++) {
        char *buf;

        buf = readline("Type a line: ");
        if (buf != NULL) cprintf("%s\n", buf);
    }
}

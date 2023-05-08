#include <inc/console.h>
#include <inc/monitor.h>
#include <inc/stdio.h>

void monitor() {
    cprintf(" ____  __   ____ \n");
    cprintf("(  __)/  \\ / ___)\n");
    cprintf(" ) _)(  O )\\___ \\\n");
    cprintf("(__)  \\__/ (____/\n");
    cprintf("\n");
}

void readline() {
    console_putc('$', COLOR_LIGHT_GREEN);
    console_putc(' ', COLOR_BLACK);
    while (1) {
    }
}

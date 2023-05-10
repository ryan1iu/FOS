#include <inc/console.h>
#include <inc/monitor.h>
#include <inc/stdio.h>

void plogo() {
    cprintf(" ____  __   ____ \n");
    cprintf("(  __)/  \\ / ___)\n");
    cprintf(" ) _)(  O )\\___ \\\n");
    cprintf("(__)  \\__/ (____/\n");
    cprintf("\n");
}

void monitor() {
    while (1) {
        readline(" ");
    }
}

#include <lib/lib.h>

void umain(int argc, char **argv) {
    pid_t proc;

    cprintf("I am the parent.  Forking the child...\n");
    if ((proc = fork()) == 0) {
        cprintf("I am the child.  Spinning...\n");
        while (1) /* do nothing */
            ;
    }

    cprintf("I am the parent.  Running the child...\n");

    sys_yield();

    cprintf("I am the parent.  Killing the child...\n");
    sys_destoryproc(proc);
}

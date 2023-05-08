#include <lib/lib.h>

void umain(int argc, char **argv) {
    cprintf("I am parent process %d\n", this_pid);
    pid_t pid = fork();
    if (pid == 0) {
        cprintf("I am child process %d\n", this_pid);
    }
}

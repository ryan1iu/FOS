#include <lib/lib.h>

#define N 128

void umain(int argc, char **argv) {
    pid_t pid;
    cprintf("I am parent process %d\n", this_pid);
    for (int n = 0; n < N; n++) {
        pid = fork();
        if (pid == 0) {
            cprintf("I am child process %d\n", this_pid);
            break;
        }
    }
}

#include <lib/lib.h>

#define N 127
static int sum = 0;

void umain(int argc, char **argv) {
    pid_t pid;
    for (int n = 0; n < N; n++) {
        pid = fork();
        if (pid == 0) {
            sum++;
            return;
        }
    }
    cprintf("sum = %d\n", sum);
}

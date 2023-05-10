#include <inc/string.h>
#include <lib/lib.h>
pid_t fork(void) {
    int res;

    pid_t child_pid;
    asm volatile("int %2" : "=a"(child_pid) : "a"(S_cowfork), "i"(T_SYSCALL));

    if (child_pid < 0) {
        cprintf("sys_fork failed");
        return -1;
    }

    if (child_pid == 0) {
        // 修改thispid指向当前子进程
        this_pid = sys_getpid();
    }
    return child_pid;
}

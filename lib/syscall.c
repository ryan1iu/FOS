#include <lib/lib.h>

static inline int32_t syscall(int n, uint32_t a1, uint32_t a2, uint32_t a3,
                              uint32_t a4, uint32_t a5) {
    int32_t ret;

    asm volatile("int %1\n"
                 : "=a"(ret)
                 : "i"(T_SYSCALL), "a"(n), "d"(a1), "c"(a2), "b"(a3), "D"(a4),
                   "S"(a5)
                 : "cc", "memory");

    return ret;
}

// 向终端输出格式化字符串
void sys_puts(const char *s, size_t len) {
    syscall(S_puts, (uint32_t)s, len, 0, 0, 0);
}

// 获取终端输入
int sys_getc(void) { return syscall(S_getc, 0, 0, 0, 0, 0); }

// 销毁进程
int sys_destoryproc(pid_t pid) {
    return syscall(S_destoryproc, pid, 0, 0, 0, 0);
}

// 获取pid
pid_t sys_getpid(void) { return syscall(S_getpid, 0, 0, 0, 0, 0); }

//
//-------------fork原语--------------------
//
int sys_mappage(pid_t src_proc, void *src_va, pid_t dst_proc, void *dst_va,
                int perm) {
    return syscall(S_mappage, src_proc, (uint32_t)src_va, dst_proc,
                   (uint32_t)dst_va, perm);
}

int sys_umappage(pid_t pid, void *va) {
    return syscall(S_umappage, pid, (uint32_t)va, 0, 0, 0);
}

int sys_allocpage(pid_t pid, void *va, int perm) {
    return syscall(S_allocpage, pid, (uint32_t)va, perm, 0, 0);
}

int sys_setstatus(pid_t pid, int st) {
    return syscall(S_setstatus, pid, st, 0, 0, 0);
}

int sys_setpfcall(pid_t pid, void *call) {
    // 注意不要传递指针
    return syscall(S_setpfcall, pid, (uint32_t)call, 0, 0, 0);
}

void sys_yield() { syscall(S_yield, 0, 0, 0, 0, 0); }

#include <inc/assert.h>
#include <inc/error.h>
#include <inc/fs.h>
#include <inc/keybd.h>
#include <inc/memlayout.h>
#include <inc/pfhandler.h>
#include <inc/pmap.h>
#include <inc/proc.h>
#include <inc/sched.h>
#include <inc/stdio.h>
#include <inc/syscall.h>

#include "inc/console.h"
#include "inc/mmu.h"

static void sys_puts(const char *s, size_t len) { cprintf("%.*s", len, s); }

static int sys_getc(void) { return kbd_getc(); }

static pid_t sys_getpid(void) { return curproc->proc_id; }

//
// 销毁一个进程
//
static int sys_destoryproc(pid_t pid) {
    int r;
    struct Proc *p;
    if (pid > NPROC) {
        panic("invalid pid: %d", pid);
    }
    if (pid == 0) {
        p = curproc;
    } else {
        p = &procs[pid - 1];
    }

    // if (p == curproc)
    //     cprintf("proc %d exit\n", curproc->proc_id);
    // else
    //     cprintf("proc %d destroy proc %d\n", curproc->proc_id, p->proc_id);
    proc_destroy(p);
    return 0;
}

static void sys_yield() { sched_yield(); }

static pid_t sys_cowfork() {
    struct Proc *parent = curproc;
    struct Proc *child = NULL;
    int res;
    if ((res = proc_alloc(&child, parent->proc_id)) < 0) {
        return res;
    }

    // 标记为不可运行
    child->proc_status = PROC_NOT_RUNABLE;

    // 复制当前进程的环境
    child->proc_tf = curproc->proc_tf;

    // 通过显示的修改子进程的eax寄存器来让子进程返回0
    child->proc_tf.tf_regs.reg_eax = 0;

    // 查询父进程中所有存在且属性为PTE_W的页表条目
    uintptr_t start = UTEXT;
    uintptr_t end = USTACKTOP;
    pte_t *pte;
    pte_t *pte_store;
    for (start; start < end; start += PGSIZE) {
        // 判断PTE是否存在
        pte = va2pte(parent->proc_pgdir, (const void *)start, 0);
        if (!pte || !(*pte & PTE_P)) {
            continue;
        }
        // 获取va所映射的page
        struct Page *page =
            vpage_lookup(parent->proc_pgdir, (void *)start, &pte_store);

        // 如果pte的属性不是pte_w 或 pte_cow ，就直接复制
        if (!(*pte & (PTE_W | PTE_COW))) {
            vpage_insert(child->proc_pgdir, page, (void *)start, *pte_store);
        }

        // 否则将父进程的pte属性改为PTE_COW
        // 同时此PTE复制到子进程中
        vpage_insert(child->proc_pgdir, page, (void *)start,
                     PTE_P | PTE_U | PTE_COW);
        vpage_insert(parent->proc_pgdir, page, (void *)start,
                     PTE_P | PTE_U | PTE_COW);
    }

    child->proc_status = PROC_RUNABLE;

    return child->proc_id;
}

//
// 系统调用，根据不同的系统调用号跳转到不同的系统调用函数执行
//
int32_t syscall(uint32_t syscallno, uint32_t a1, uint32_t a2, uint32_t a3,
                uint32_t a4, uint32_t a5) {
    switch (syscallno) {
        case S_puts:
            sys_puts((const char *)a1, (size_t)a2);
            return 0;
            break;
        case S_getc:
            return sys_getc();
            break;
        case S_getpid:
            return sys_getpid();
            break;
        case S_destoryproc:
            return sys_destoryproc((pid_t)a1);
            break;
        case S_cowfork:
            return sys_cowfork();
            break;
        case S_yield:
            sys_yield();
            return 0;
            break;
        case S_getfid:
            return fs_getfid((const char *)a1);
            break;
        default:
            return -E_INVAL;
    }

    // 正常情况下不会到达这里
    return -1;
}

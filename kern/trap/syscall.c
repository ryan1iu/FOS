#include <inc/assert.h>
#include <inc/error.h>
#include <inc/memlayout.h>
#include <inc/pfhandler.h>
#include <inc/pmap.h>
#include <inc/proc.h>
#include <inc/stdio.h>
#include <inc/syscall.h>

static void sys_puts(const char *s, size_t len) { cprintf("%.*s", len, s); }

static int sys_getc(void) { return 'e'; }

static pid_t sys_getpid(void) { return curproc->proc_id; }

//
// 销毁一个进程
//
static int sys_destoryproc(pid_t pid) {
    int r;
    struct Proc *p;
    p = &procs[pid - 1];

    // if (p == curproc)
    //     cprintf("proc %d exit\n", curproc->proc_id);
    // else
    //     cprintf("proc %d destroy proc %d\n", curproc->proc_id, p->proc_id);
    proc_destroy(p);
    return 0;
}

//
// 设置进程的page fault处理函数
//
static int sys_setpfcall(pid_t pid, void *call) {
    struct Proc *proc;
    if (pid > NPROC) {
        panic("invalid pid");
    }
    proc = &procs[pid - 1];
    proc->proc_pfcall = call;
    return 0;
}

// Allocate a page of memory and map it at 'va' with permission
// 'perm' in the address space of 'envid'.
// The page's contents are set to 0.
// If a page is already mapped at 'va', that page is unmapped as a
// side effect.
static int sys_allocpage(pid_t pid, void *va, int perm) {
    struct Proc *proc;

    if (pid > NPROC) {
        panic("invalid pid");
    }

    if (pid == 0) {
        proc = curproc;
    } else {
        proc = &procs[pid - 1];
    }

    // if va >= UTOP or not page page-aligned
    uint32_t uva = (uint32_t)va;
    if (uva >= UTOP || (uva % PGSIZE) != 0) {
        return -E_INVAL;
    }

    // 如果perm中存在除PTE_SYSCALL之外的位
    if ((perm & (~PTE_SYSCALL)) != 0) {
        return -E_INVAL;
    }

    // 如果perm中没有PTE_P | PTE_U
    if ((perm & PTE_P) == 0 || (perm & PTE_U) == 0) {
        return -E_INVAL;
    }

    // if no memory
    struct Page *page;
    page = ppage_alloc(1);
    if (page == NULL) {
        return -E_NO_MEM;
    }

    int res;
    res = vpage_insert(proc->proc_pgdir, page, va, perm);
    if (res < 0) {
        return res;
    }

    return 0;
}

// Map the page of memory at 'srcva' in srcenvid's address space
// at 'dstva' in dstenvid's address space with permission 'perm'.
// Perm has the same restrictions as in sys_page_alloc, except
// that it also must not grant write access to a read-only
// page.
static int sys_mappage(pid_t srcpid, void *src_va, pid_t dstpid, void *dst_va,
                       int perm) {
    // 检查pid的合法性
    if (srcpid > NPROC) {
        panic("invalid srcpid");
    }
    if (dstpid > NPROC) {
        panic("invalid dstpid");
    }
    struct Proc *srcproc;
    struct Proc *dstproc;

    if (srcpid == 0) {
        srcproc = curproc;
    } else {
        srcproc = &procs[srcpid - 1];
    }

    if (dstpid == 0) {
        dstproc = curproc;
    } else {
        dstproc = &procs[dstpid - 1];
    }

    // srcva 和 dstva 的合法性
    uint32_t usrcva = (uint32_t)src_va;
    uint32_t udstva = (uint32_t)dst_va;
    if (usrcva >= UTOP || (usrcva % PGSIZE) != 0) {
        return -E_INVAL;
    }
    if (udstva >= UTOP || (udstva % PGSIZE) != 0) {
        return -E_INVAL;
    }

    // 如果perm中存在除PTE_SYSCALL之外的位
    if ((perm & (~PTE_SYSCALL)) != 0) {
        return -E_INVAL;
    }

    // 如果perm中没有PTE_P | PTE_U
    if ((perm & PTE_P) == 0 || (perm & PTE_U) == 0) {
        return -E_INVAL;
    }

    pte_t *perm_store;
    struct Page *srcpage;
    srcpage = vpage_lookup(srcproc->proc_pgdir, src_va, &perm_store);

    // page_lookup 返回NULL，没有与srcva对应的page
    if (srcpage == NULL) {
        return -E_INVAL;
    }

    if ((perm & PTE_W) && !(*perm_store & PTE_W)) {
        return -E_INVAL;
    }

    int res;
    res = vpage_insert(dstproc->proc_pgdir, srcpage, dst_va, perm);
    if (res < 0) {
        // E_NO_MEM
        return res;
    }

    return 0;
}

// Unmap the page of memory at 'va' in the address space of 'envid'.
// If no page is mapped, the function silently succeeds.
static int sys_umappage(pid_t pid, void *va) {
    uint32_t uva = (uint32_t)va;
    if (uva >= UTOP) {
        return -E_INVAL;
    }

    struct Proc *proc;
    if (pid == 0) {
        proc = curproc;
    } else {
        proc = &procs[pid - 1];
    }

    if (pid > NPROC) {
        panic("invalid pid");
    }

    vpage_remove(proc->proc_pgdir, va);

    return 0;
}

static pid_t sys_fork() {
    struct Proc *child_proc;

    int res = proc_alloc(&child_proc, curproc->proc_id);
    if (res < 0) {
        return res;
    }

    // 标记为不可运行
    child_proc->proc_status = PROC_NOT_RUNABLE;

    // 复制当前进程的环境
    child_proc->proc_tf = curproc->proc_tf;

    // 通过显示的修改子进程的eax寄存器来让子进程返回0
    child_proc->proc_tf.tf_regs.reg_eax = 0;

    // 返回子进程的id
    return child_proc->proc_id;
}

static int sys_setstatus(pid_t pid, int status) {
    if (pid > NPROC) {
        panic("invalid pid");
    }
    struct Proc *proc;
    proc = &procs[pid - 1];
    proc->proc_status = status;
    return 0;
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
        case S_setpfcall:
            return sys_setpfcall((pid_t)a1, (void *)a2);
            break;
        case S_destoryproc:
            return sys_destoryproc((pid_t)a1);
            break;
        case S_mappage:
            return sys_mappage((pid_t)a1, (void *)a2, (pid_t)a3, (void *)a4,
                               (int)a5);
            break;
        case S_umappage:
            return sys_umappage((pid_t)a1, (void *)a2);
            break;
        case S_allocpage:
            return sys_allocpage((pid_t)a1, (void *)a2, (int)a3);
            break;
        case S_fork:
            return sys_fork((pid_t)a1);
            break;
        case S_setstatus:
            return sys_setstatus((pid_t)a1, (int)a2);
            break;
        default:
            return -E_INVAL;
    }

    // 正常情况下不会到达这里
    return -1;
}

#include <inc/string.h>
#include <lib/lib.h>

// PTE_COW marks copy-on-write page table entries.
// It is one of the bits explicitly allocated to user processes (PTE_AVAIL).
#define PTE_COW 0x800
extern void _pf_call(void);
extern volatile pte_t uvpt[];  // VA of "virtual page table"
extern volatile pde_t uvpd[];  // VA of current page directory

//
// Custom page fault handler - if faulting page is copy-on-write,
// map in our own private writable copy.
//
static void pgfault(struct UTrapframe *utf) {
    void *addr = (void *)utf->utf_fault_va;
    uint32_t err = utf->utf_err;

    // Check that the faulting access was (1) a write, and (2) to a
    // copy-on-write page.
    if (!(err & FEC_WR) || !(uvpt[PGNUM(addr)] & PTE_COW)) {
        cprintf("fault isn't FEC_WR");
    }

    // Allocate a new page, map it at a temporary location (PFTEMP),
    // copy the data from the old page to the new page, then move the new
    // page to the old page's address.
    sys_allocpage(0, (void *)PFTEMP, PTE_P | PTE_U | PTE_W);

    addr = ROUNDDOWN(addr, PGSIZE);
    memcpy((void *)PFTEMP, addr, PGSIZE);

    sys_mappage(0, (void *)PFTEMP, 0, addr, PTE_P | PTE_U | PTE_W);
    sys_umappage(0, (void *)PFTEMP);
}

//
// Map our virtual page pn (address pn*PGSIZE) into the target envid
// at the same virtual address.  If the page is writable or copy-on-write,
// the new mapping must be created copy-on-write, and then our mapping must be
// marked copy-on-write as well.  (Exercise: Why do we need to mark ours
// copy-on-write again if it was already copy-on-write at the beginning of
// this function?)
//
// Returns: 0 on success, < 0 on error.
// It is also OK to panic on error.
//
static int duppage(pid_t child_pid, unsigned pn) {
    int r;
    int perm = PTE_P | PTE_U | PTE_COW;
    if (uvpt[pn] & (PTE_W | PTE_COW)) {
        sys_mappage(this_pid, (void *)(pn * PGSIZE), child_pid,
                    (void *)(pn * PGSIZE), perm);
        sys_mappage(this_pid, (void *)(pn * PGSIZE), this_pid,
                    (void *)(pn * PGSIZE), perm);
    } else {
        sys_mappage(this_pid, (void *)(pn * PGSIZE), child_pid,
                    (void *)(pn * PGSIZE), PTE_U | PTE_P);
    }
    return 0;
}

//
// User-level fork with copy-on-write.
// Set up our page fault handler appropriately.
// Create a child.
// Copy our address space and page fault handler setup to the child.
// Then mark the child as runnable and return.
//
// Returns: child's envid to the parent, 0 to the child, < 0 on error.
// It is also OK to panic on error.
pid_t fork(void) {
    int res;

    // 设置父进程的page fault handler
    pf_set(pgfault);

    // 调用sys_exofork创建原始子进程
    pid_t child_pid;
    asm volatile("int %2" : "=a"(child_pid) : "a"(S_fork), "i"(T_SYSCALL));

    if (child_pid < 0) {
        cprintf("sys_fork failed");
        return -1;
    }

    if (child_pid == 0) {
        // 修改thispid指向当前子进程
        this_pid = sys_getpid();
        return 0;

    } else {
        // 遍历父进程的地址空间，将标记为PTE_W或PTE_COW的页面复制到子进程的地址空间
        int pn;
        for (pn = PGNUM(UTEXT); pn < PGNUM(USTACKTOP); pn++) {
            if ((uvpd[pn >> 10] & PTE_P) && (uvpt[pn] & PTE_P)) {
                // 页表
                duppage(child_pid, pn);
            }
        }

        // 为子进程分配异常栈
        sys_allocpage(child_pid, (void *)(UXSTACKTOP - PGSIZE),
                      PTE_P | PTE_U | PTE_W);

        // 设置子进程的page fault handler
        // 这里有一个细节，因为父进程已经调用过set_pgfault_handler了
        // 所以子进程如果再次调用不会修改其env->pgfault_upcall,所以需要
        // 直接调用系统调用来修改
        sys_setpfcall(child_pid, _pf_call);
        // 修改子进程状态为可运行
        sys_setstatus(child_pid, PROC_RUNABLE);
    }
    return child_pid;
}

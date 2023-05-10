#include <inc/assert.h>
#include <inc/memlayout.h>
#include <inc/mmu.h>
#include <inc/pfhandler.h>
#include <inc/pmap.h>
#include <inc/proc.h>
#include <inc/stdio.h>
#include <inc/string.h>
#include <inc/x86.h>

void handle_pgfault(struct Trapframe *tf) {
    uint32_t fault_va;
    uint32_t error = tf->tf_err;

    fault_va = rcr2();

    if ((tf->tf_cs & 3) == 0) {
        // 如何page fault发生在内核中直接终止运行
        panic("page fault in kernel! falut_va:%x\n", fault_va);
    }

    pte_t *pte;
    pte = va2pte(curproc->proc_pgdir, (const void *)fault_va, 0);
    if (pte == NULL) {
        cprintf("invalid address %x\n", fault_va);
    } else if (!(error & FEC_WR) || !(*pte & PTE_COW)) {
        cprintf("fault isn't FEC_WR");
    } else {
        struct Page *new_page = ppage_alloc(1);
        vpage_insert(curproc->proc_pgdir, new_page, (void *)PFTEMP,
                     PTE_P | PTE_U | PTE_W);

        fault_va = ROUNDDOWN(fault_va, PGSIZE);
        memcpy((void *)PFTEMP, (void *)fault_va, PGSIZE);

        vpage_insert(curproc->proc_pgdir, new_page, (void *)fault_va,
                     PTE_P | PTE_W | PTE_U);
        vpage_remove(curproc->proc_pgdir, (void *)PFTEMP);

        proc_run(curproc);
    }

    cprintf("[%08x] user fault va %08x ip %08x\n", curproc->proc_id, fault_va,
            tf->tf_eip);

    proc_destroy(curproc);
}

#include <inc/assert.h>
#include <inc/memlayout.h>
#include <inc/mmu.h>
#include <inc/pfhandler.h>
#include <inc/proc.h>
#include <inc/x86.h>

void handle_pgfault(struct Trapframe *tf) {
    uint32_t fault_va;

    fault_va = rcr2();

    if ((tf->tf_cs & 3) == 0) {
        // 如何page fault发生在内核中直接终止运行
        panic("page fault in kernel! falut_va:%x\n", fault_va);
    }

    struct UTrapframe *frame;
    uintptr_t stack_top;
    stack_top = tf->tf_esp;
    if (stack_top > UXSTACKTOP - PGSIZE && stack_top < UXSTACKTOP) {
        // 说明page fault发生在upcall中
        frame =
            (struct UTrapframe *)(stack_top - sizeof(struct UTrapframe) - 4);
    } else {
        frame = (struct UTrapframe *)(UXSTACKTOP - sizeof(struct UTrapframe));
    }

    if (curproc->proc_pfcall) {
        frame->utf_fault_va = fault_va;
        frame->utf_err = tf->tf_err;
        frame->utf_regs = tf->tf_regs;
        frame->utf_eip = tf->tf_eip;
        frame->utf_eflags = tf->tf_eflags;
        frame->utf_esp = tf->tf_esp;

        // 修改eip使得函数返回到page fault处理函数中
        curproc->proc_tf.tf_eip = (uintptr_t)curproc->proc_pfcall;

        // 修改esp以使处理函数运行在异常栈中
        curproc->proc_tf.tf_esp = (uintptr_t)frame;
        proc_run(curproc);
    }

    cprintf("[%08x] user fault va %08x ip %08x\n", curproc->proc_id, fault_va,
            tf->tf_eip);

    proc_destroy(curproc);
}

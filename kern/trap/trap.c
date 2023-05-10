#include <inc/assert.h>
#include <inc/console.h>
#include <inc/error.h>
#include <inc/keybd.h>
#include <inc/memlayout.h>
#include <inc/mmu.h>
#include <inc/monitor.h>
#include <inc/pfhandler.h>
#include <inc/proc.h>
#include <inc/sched.h>
#include <inc/syscall.h>
#include <inc/trap.h>
#include <inc/x86.h>

#include "inc/stdio.h"

// task state segment
static struct Taskstate ts;

// 终端描述符表
struct Gatedesc idt[256] = {{0}};
struct Pseudodesc idt_pd = {sizeof(idt) - 1, (uint32_t)idt};

void trap_init(void) {
    // 设置任务段
    ts.ts_esp0 = KSTACKTOP;
    ts.ts_ss0 = GD_KD;
    ts.ts_iomb = sizeof(struct Taskstate);

    gdt[GD_TSS0 >> 3] =
        SEG16(STS_T32A, (uint32_t)(&ts), sizeof(struct Taskstate) - 1, 0);
    gdt[GD_TSS0 >> 3].sd_s = 0;

    // 加载TSS段选择子
    ltr(GD_TSS0);

    void divide_error_handler();
    void debug_exception_handler();
    void non_maskable_interrupt_handler();
    void breakpoint_handler();
    void overflow_handler();
    void bounds_check_handler();
    void invalid_opcode_handler();
    void device_not_available_handler();
    void double_fault_handler();
    void invalid_tss_handler();
    void segment_not_present_handler();
    void stack_exception_handler();
    void general_protection_fault_handler();
    void pagefault_handler();
    void floating_point_error_handler();
    void alignment_check_handler();
    void machine_check_handler();
    void simd_floating_point_error_handler();
    void syscall_handler();
    void timer_handler();
    void kbd_handler();

    // 设置终端描述符条目
    SETGATE(idt[T_DIVIDE], 0, GD_KT, divide_error_handler, 0);
    SETGATE(idt[T_DEBUG], 0, GD_KT, debug_exception_handler, 0);
    SETGATE(idt[T_NMI], 0, GD_KT, non_maskable_interrupt_handler, 0);
    SETGATE(idt[T_BRKPT], 0, GD_KT, breakpoint_handler, 3);
    SETGATE(idt[T_OFLOW], 0, GD_KT, overflow_handler, 0);
    SETGATE(idt[T_BOUND], 0, GD_KT, bounds_check_handler, 0);
    SETGATE(idt[T_ILLOP], 0, GD_KT, invalid_opcode_handler, 0);
    SETGATE(idt[T_DEVICE], 0, GD_KT, device_not_available_handler, 0);
    SETGATE(idt[T_DBLFLT], 0, GD_KT, double_fault_handler, 0);
    SETGATE(idt[T_TSS], 0, GD_KT, invalid_tss_handler, 0);
    SETGATE(idt[T_SEGNP], 0, GD_KT, segment_not_present_handler, 0);
    SETGATE(idt[T_STACK], 0, GD_KT, stack_exception_handler, 0);
    SETGATE(idt[T_GPFLT], 0, GD_KT, general_protection_fault_handler, 0);
    SETGATE(idt[T_PGFLT], 0, GD_KT, pagefault_handler, 0);
    SETGATE(idt[T_FPERR], 0, GD_KT, floating_point_error_handler, 0);
    SETGATE(idt[T_ALIGN], 0, GD_KT, alignment_check_handler, 0);
    SETGATE(idt[T_MCHK], 0, GD_KT, machine_check_handler, 0);
    SETGATE(idt[T_SIMDERR], 0, GD_KT, simd_floating_point_error_handler, 0);
    // 注意这里将DPL设为3，即用户级别
    SETGATE(idt[T_SYSCALL], 0, GD_KT, syscall_handler, 3);
    SETGATE(idt[IRQ_OFFSET + IRQ_TIMER], 0, GD_KT, timer_handler, 0);
    SETGATE(idt[IRQ_OFFSET + IRQ_KBD], 1, GD_KT, kbd_handler, 0);

    // 加载IDT
    lidt(&idt_pd);
}

void print_trapframe(struct Trapframe *tf) {
    cprintf(">>>goto trap %d\n", tf->tf_trapno);
}

void trap(struct Trapframe *tf) {
    // 检查是否禁用中断
    // assert(!(read_eflags() & FL_IF));

    if ((tf->tf_cs & 3) == 3) {
        // 中断发生在用户模式
        assert(curproc);

        // 保存进程上下文状态以供中断结束后恢复
        curproc->proc_tf = *tf;
        tf = &curproc->proc_tf;
    }

    // 检查中断号并跳转到对应的处理程序
    switch (tf->tf_trapno) {
        // 系统调用
        case T_SYSCALL:
            // 这里注意要将返回值手动写入eax寄存器
            tf->tf_regs.reg_eax = syscall(
                tf->tf_regs.reg_eax, tf->tf_regs.reg_edx, tf->tf_regs.reg_ecx,
                tf->tf_regs.reg_ebx, tf->tf_regs.reg_edi, tf->tf_regs.reg_esi);
            break;
            // 断点异常
        case T_BRKPT:
            monitor();
            break;
            // Page fault
        case T_PGFLT:
            handle_pgfault(tf);
            break;

        case IRQ_OFFSET + IRQ_TIMER:
            // 发送普通EOI命令
            outb(0x20, 0x20);
            sched_yield();
            break;

        case IRQ_OFFSET + IRQ_KBD:
            kbd_intr();
            break;
        default:
            print_trapframe(tf);
            if (tf->tf_cs == GD_KT)
                panic("unhandled trap in kernel");
            else {
                // 如果在proc_destroy中出现错误，就会产生递归调用，
                // 因此必须保证proc_destroy的正确性
                proc_destroy(curproc);
                return;
            }
            break;
    }
    // 恢复进程的上下文环境
    if (curproc && curproc->proc_status == PROC_RUNNING)
        proc_run(curproc);
    else
        // 如果当前没有进程在运行就调用sched函数
        sched_yield();
}

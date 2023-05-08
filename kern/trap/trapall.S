#include <inc/mmu.h>
#include <inc/memlayout.h>
#include <inc/trap.h>

#define TRAPHANDLER(name, num)						\
	.globl name;		/* define global symbol for 'name' */	\
	.type name, @function;	/* symbol type is function */		\
	.align 2;		/* align function definition */		\
	name:			/* function starts here */		\
	pushl $(num);							\
	jmp _alltraps


#define TRAPHANDLER_NOEC(name, num)					\
	.globl name;							\
	.type name, @function;						\
	.align 2;							\
	name:								\
	pushl $0;							\
	pushl $(num);							\
	jmp _alltraps

.text
TRAPHANDLER_NOEC(divide_error_handler, T_DIVIDE)
TRAPHANDLER_NOEC(debug_exception_handler, T_DEBUG)
TRAPHANDLER_NOEC(non_maskable_interrupt_handler, T_NMI)
TRAPHANDLER_NOEC(breakpoint_handler, T_BRKPT)
TRAPHANDLER_NOEC(overflow_handler, T_OFLOW)
TRAPHANDLER_NOEC(bounds_check_handler, T_BOUND)
TRAPHANDLER_NOEC(invalid_opcode_handler, T_ILLOP)
TRAPHANDLER_NOEC(device_not_available_handler, T_DEVICE)
TRAPHANDLER(double_fault_handler, T_DBLFLT)
TRAPHANDLER(invalid_tss_handler, T_TSS)
TRAPHANDLER(segment_not_present_handler, T_SEGNP)
TRAPHANDLER(stack_exception_handler, T_STACK)
TRAPHANDLER(general_protection_fault_handler, T_GPFLT)
TRAPHANDLER(pagefault_handler, T_PGFLT)
TRAPHANDLER_NOEC(floating_point_error_handler, T_FPERR)
TRAPHANDLER(alignment_check_handler, T_ALIGN)
TRAPHANDLER_NOEC(machine_check_handler, T_MCHK)
TRAPHANDLER_NOEC(simd_floating_point_error_handler, T_SIMDERR)
TRAPHANDLER_NOEC(syscall_handler, T_SYSCALL)

 _alltraps:
// push ds and es and general registers
push %ds
push %es
pushal

// load ds and es with GD_KD, for kernel stack locates in data
mov $GD_KD, %ax
mov %ax, %ds
mov %ax, %es

// pass tf as an argument
pushl %esp

// call trap and no need to return
call trap

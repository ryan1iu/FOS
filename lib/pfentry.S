#include <inc/mmu.h>
#include <inc/memlayout.h>

// Page fault upcall entrypoint.

.text
.globl _pf_call
_pf_call:
	// Call the C page fault handler.
	pushl %esp			// function argument: pointer to UTF
	movl _pf_handler, %eax
	call *%eax
	addl $4, %esp			// pop function argument
	
	
    movl 0x28(%esp), %eax    // 将eip存放到eax中
    subl $0x4, 0x30(%esp)    // 将trap-time esp减4来存放eip
    movl 0x30(%esp), %ebp
    movl %eax, (%ebp)

	// Restore the trap-time registers.  After you do this, you
	// can no longer modify any general-purpose registers.
    popl %eax    // pop掉fault_va 和 err
    popl %eax
    popal

	// Restore eflags from the stack.  After you do this, you can
	// no longer use arithmetic operations or anything else that
	// modifies eflags.
    addl $0x4, %esp
    popfl

	// Switch back to the adjusted trap-time stack.
    popl %esp


	// Return to re-execute the instruction that faulted.
    ret

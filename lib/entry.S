#include <inc/mmu.h>
#include <inc/memlayout.h>


.text
.globl _start
_start:
	// See if we were started with arguments on the stack
	cmpl $USTACKTOP, %esp
	jne args_exist

	pushl $0
	pushl $0

args_exist:
	call gomain
1:	jmp 1b

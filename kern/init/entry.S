#include <inc/mmu.h>
#include <inc/memlayout.h>


# 内核的链接地址为 KERNBASE ，加载地址（物理地址）为1MB
# RELOC(x) 用来将 x 从链接地址转换为加载地址
# 通过采用临时的分页机制，将虚拟地址空间中的0x00000000到0x00400000以及
# 0xf0000000到0xf0400000映射到从0x00000000到0x00400000的物理地址空间

#define	RELOC(x) ((x) - KERNBASE)

.text
# entry point
# 因为刚进入内核时还没有开启映射，所以内核运行在“物理地址”上，
# 需要将入口地址转为物理地址，并设置链接选项
.globl		_start
_start = RELOC(entry)

.globl entry
entry:
	movw	$0x1234,0x472			# warm boot

# 加载临时页表，来完成临时映射，同时开启分页机制
	movl	$(RELOC(entry_pgdir)), %eax
	movl	%eax, %cr3
	# 开启分页机制
	movl	%cr0, %eax
	orl	$(CR0_PE|CR0_PG|CR0_WP), %eax
	movl	%eax, %cr0

# 跳转到KERNBASE之上
	mov	$relocated, %eax
	jmp	*%eax
relocated:

# 清空ebp，为backtrace提供一个结束标志
	movl	$0x0,%ebp

	# 设置临时内核栈
	movl	$(bootstacktop),%esp

	# 跳转到C代码
	call	kern_init

# 如果错误则不断循环
spin:	jmp	spin


.data
# boot stack
	.p2align	PGSHIFT		# 强制页对齐
	.globl		bootstack
bootstack:
	.space		KSTKSIZE  # 指定kern stack size
	.globl		bootstacktop   
bootstacktop:


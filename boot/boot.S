#include <inc/mmu.h>

# 从实模式切换到保护模式，跳转到C程序
# BIOS会将这段代码从硬盘加载到起始位置为0x7c00的内存中，然后在
# 实模式中开始执行（%cs=0， %ip=7c00)

.set PROT_MODE_CSEG, 0x8         # 内核代码段选择子
.set PROT_MODE_DSEG, 0x10        # 内核数据段选择子
.set CR0_PE_ON,      0x1         # 保护模式开关

.globl start
start:
  .code16                     # 指示汇编器以实模式方式工作
  cli                         # 关闭中断
  cld                         # 设置DF=0，字符操作递增

  # 将ds,es,ss置零
  xorw    %ax,%ax             
  movw    %ax,%ds            
  movw    %ax,%es           
  movw    %ax,%ss          

  # 开启A20
seta20.1:
  inb     $0x64,%al               # Wait for not busy
  testb   $0x2,%al
  jnz     seta20.1

  movb    $0xd1,%al               # 0xd1 -> port 0x64
  outb    %al,$0x64

seta20.2:
  inb     $0x64,%al               # Wait for not busy
  testb   $0x2,%al
  jnz     seta20.2

  movb    $0xdf,%al               # 0xdf -> port 0x60
  outb    %al,$0x60

  # 加载自定义的GDT，使得虚拟地址直接映射到物理地址
  lgdt    gdtdesc
  movl    %cr0, %eax
  orl     $CR0_PE_ON, %eax        # 开启保护模式
  movl    %eax, %cr0
  
  # 将CPU切换到32位模式
  ljmp    $PROT_MODE_CSEG, $protcseg

  .code32                     
protcseg:
  # 因为不使用分段功能，所以将除CS外的段寄存器都设置为可读的，地址范围为0~4GB的空间。
  movw    $PROT_MODE_DSEG, %ax    
  movw    %ax, %ds               
  movw    %ax, %es              
  movw    %ax, %fs             
  movw    %ax, %gs            
  movw    %ax, %ss           
  
  # 为bootmain函数设置一个初始栈空间
  movl    $start, %esp
  call bootmain

  # 如果bootmain失败返回，就一直循环
spin:
  jmp spin

# Bootstrap GDT
.p2align 2                                # 强制4字节对齐
gdt:
  SEG_NULL				# null seg
  SEG(STA_X|STA_R, 0x0, 0xffffffff)	# code seg
  SEG(STA_W, 0x0, 0xffffffff)	        # data seg

# GDT的大小和地址，用来提供给lgdt指令
gdtdesc:
  .word   0x17                            # sizeof(gdt) - 1
  .long   gdt                             # address gdt


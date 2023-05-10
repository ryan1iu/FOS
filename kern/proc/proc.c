#include <inc/assert.h>
#include <inc/elf.h>
#include <inc/error.h>
#include <inc/memlayout.h>
#include <inc/mmu.h>
#include <inc/pmap.h>
#include <inc/proc.h>
#include <inc/sched.h>
#include <inc/string.h>
#include <inc/types.h>
#include <inc/x86.h>

struct Proc *procs = NULL;    // 所有的进程
struct Proc *curproc = NULL;  // 当前占据CPU的进程

static struct Proc *proc_free_list;  // 空闲进程链表

// 全局描述符表
// 这里只利用其权限分隔功能，内核段和用户段只有DPL位不同
// 内核段的DPL为0，用户段的DPL为3
struct Segdesc gdt[] = {
    // 0x0 - unused (always faults -- for trapping NULL far pointers)
    SEG_NULL,

    // 0x8 - 内核代码段
    [GD_KT >> 3] = SEG(STA_X | STA_R, 0x0, 0xffffffff, 0),

    // 0x10 - 内核数据段
    [GD_KD >> 3] = SEG(STA_W, 0x0, 0xffffffff, 0),

    // 0x18 - 用户代码段
    [GD_UT >> 3] = SEG(STA_X | STA_R, 0x0, 0xffffffff, 3),

    // 0x20 - 用户数据段
    [GD_UD >> 3] = SEG(STA_W, 0x0, 0xffffffff, 3),

    // 0x28 - TSS段
    [GD_TSS0 >> 3] = SEG_NULL};

struct Pseudodesc gdt_pd = {sizeof(gdt) - 1, (unsigned long)gdt};

// 初始化proc_free_list
void proc_init(void) {
    int i;
    // 确保最小的 Proc 在最前端
    for (i = NPROC - 1; i >= 0; --i) {
        procs[i].proc_id = 0;

        procs[i].proc_link = proc_free_list;
        proc_free_list = &procs[i];
    }
    // 加载全局描述符表
    lgdt(&gdt_pd);
    // The kernel never uses GS or FS, so we leave those set to
    // the user data segment.
    asm volatile("movw %%ax,%%gs" : : "a"(GD_UD | 3));
    asm volatile("movw %%ax,%%fs" : : "a"(GD_UD | 3));
    // The kernel does use ES, DS, and SS.  We'll change between
    // the kernel and user data segments as needed.
    asm volatile("movw %%ax,%%es" : : "a"(GD_KD));
    asm volatile("movw %%ax,%%ds" : : "a"(GD_KD));
    asm volatile("movw %%ax,%%ss" : : "a"(GD_KD));
    // Load the kernel text segment into CS.
    asm volatile("ljmp %0,$1f\n 1:\n" : : "i"(GD_KT));
    lldt(0);
}

//
// 申请并初始化一个新的进程
//
int proc_alloc(struct Proc **newproc_store, pid_t parent_id) {
    // 申请一个空闲进程
    struct Proc *p;
    if (proc_free_list == NULL) {
        cprintf("no free proc\n");
        return -E_NO_FREE_PROC;
    }

    p = proc_free_list;

    // 初始化进程地址空间
    // 为进程申请一个页目录表
    struct Page *pgdir = NULL;
    pgdir = ppage_alloc(1);
    if (pgdir == NULL) {
        return -E_NO_MEM;
    }

    // 获取result的虚拟地址
    char *page_va;
    page_va = page2kva(pgdir);

    // 复制内核地址空间布局,因为用户地址空间在UTOP以上跟内核地址空间保持一致
    memcpy(page_va, kern_pgdir, PGSIZE);

    // 设置进程页目录的内核虚拟地址
    p->proc_pgdir = (pde_t *)(page2kva(pgdir));

    // 初始化进程id为其在procs中的index
    p->proc_id = (p - procs) + 1;
    p->proc_parent_id = parent_id;
    p->proc_status = PROC_RUNABLE;

    // 清理残留的上下文环境
    memset(&p->proc_tf, 0, sizeof(p->proc_tf));

    // Set up appropriate initial values for the segment registers.
    // GD_UD is the user data segment selector in the GDT, and
    // GD_UT is the user text segment selector (see inc/memlayout.h).
    // The low 2 bits of each segment register contains the
    // Requestor Privilege Level (RPL); 3 means user mode.  When
    // we switch privilege levels, the hardware does various
    // checks involving the RPL and the Descriptor Privilege Level
    // (DPL) stored in the descriptors themselves.
    p->proc_tf.tf_ds = GD_UD | 3;
    p->proc_tf.tf_es = GD_UD | 3;
    p->proc_tf.tf_ss = GD_UD | 3;
    p->proc_tf.tf_esp = USTACKTOP;
    p->proc_tf.tf_cs = GD_UT | 3;

    // 设置IF以允许外部中断
    p->proc_tf.tf_eflags |= FL_IF;

    proc_free_list = p->proc_link;
    p->proc_link = NULL;

    *newproc_store = p;

    return 0;
}

//
// 为进程申请len个字节的物理地址空间并将其映射到以va起始的虚拟地址空间中
//
static void proc_region_alloc(struct Proc *p, void *va, size_t len) {
    uintptr_t va_start = (uintptr_t)ROUNDDOWN(va, PGSIZE);
    uintptr_t va_end = (uintptr_t)ROUNDUP(va + len, PGSIZE);

    for (; va_start < va_end; va_start += PGSIZE) {
        // 申请一个物理页
        struct Page *page;
        page = ppage_alloc(1);
        if (!page) {
            panic("ppage_alloc failed!");
        }
        // 将物理页映射到va_start起始的虚拟地址空间
        if (vpage_insert(p->proc_pgdir, page, (char *)va_start,
                         PTE_W | PTE_U | PTE_P) < 0) {
            panic("vpage_insert failed!");
        }
    }
}

//
// Set up the initial program binary, stack, and processor flags
// for a user process.
static void load_icode(struct Proc *p, uint8_t *binary) {
    // 切换为用户虚拟地址空间，以便memcpy能够方便的移动数据
    lcr3(PADDR(p->proc_pgdir));

    struct Elf *elf = (struct Elf *)binary;
    // 检查 elf magic
    if (elf->e_magic != ELF_MAGIC) {
        panic("invalid elf format!");
    }

    struct Proghdr *ph, *eph;

    // 获取程序头表的起始和结束地址
    ph = (struct Proghdr *)(binary + elf->e_phoff);
    eph = ph + elf->e_phnum;
    for (; ph < eph; ph++) {
        // 如果这个段是可加载的
        if (ph->p_type == ELF_PROG_LOAD) {
            // 从该段指定的VMA处申请空间，将段中数据复制过去
            proc_region_alloc(p, (char *)ph->p_va, ph->p_memsz);
            memcpy((char *)ph->p_va, (char *)binary + ph->p_offset,
                   ph->p_filesz);
        }
    }

    // 设置进程入口地址
    p->proc_tf.tf_eip = elf->e_entry;

    // 加载完毕，切换到内核地址空间
    lcr3(PADDR(kern_pgdir));

    // 申请栈空间
    struct Page *stack_page = ppage_alloc(1);
    if (!stack_page) {
        panic("out of memory when alloc process stack");
    }
    if (vpage_insert(p->proc_pgdir, stack_page, (char *)(USTACKTOP - PGSIZE),
                     PTE_W | PTE_U | PTE_P) < 0) {
        panic("failed to set process stack");
    }
}

//
// 创建一个新的进程
//
void proc_create(uint8_t *binary) {
    struct Proc *newproc;
    int ret;
    if ((ret = proc_alloc(&newproc, 0)) < 0) {
        panic("proc_create: %e\n", ret);
    }
    load_icode(newproc, binary);
}

//
// 释放一个进程，清空其内存空间
//
void proc_destroy(struct Proc *p) {
    pte_t *pt;
    uint32_t pdeno, pteno;
    physaddr_t pa;

    // If freeing the current Procironment, switch to kern_pgdir
    // before freeing the page directory, just in case the page
    // gets reused.
    if (p == curproc) lcr3(PADDR(kern_pgdir));

    // Flush all mapped pages in the user portion of the address space
    for (pdeno = 0; pdeno < PDX(UTOP); pdeno++) {
        // only look at mapped page tables
        if (!(p->proc_pgdir[pdeno] & PTE_P)) continue;

        // find the pa and va of the page table
        pa = PTE_ADDR(p->proc_pgdir[pdeno]);
        pt = (pte_t *)KADDR(pa);

        // unmap all PTEs in this page table
        for (pteno = 0; pteno <= PTX(~0); pteno++) {
            if (pt[pteno] & PTE_P)
                vpage_remove(p->proc_pgdir, PGADDR(pdeno, pteno, 0));
        }

        // free the page table itself
        p->proc_pgdir[pdeno] = 0;
        ppage_decref(pa2page(pa));
    }

    // free the page directory
    pa = PADDR(p->proc_pgdir);
    p->proc_pgdir = 0;
    ppage_decref(pa2page(pa));

    // return the Procironment to the free list
    p->proc_status = PROC_FREE;
    p->proc_link = proc_free_list;
    proc_free_list = p;

    cprintf("destory %d success\n", proc_free_list->proc_id);
    // cprintf("free proc %d success\n", p->proc_id);
    sched_yield();
}

//
// Restores the register values in the Trapframe with the 'iret'
// instruction. This exits the kernel and starts executing some
// Procironment's code.
//
void proc_pop_tf(struct Trapframe *tf) {
    asm volatile(
        "\tmovl %0,%%esp\n"
        "\tpopal\n"
        "\tpopl %%es\n"
        "\tpopl %%ds\n"
        "\taddl $0x8,%%esp\n" /* skip tf_trapno and tf_errcode */
        "\tiret\n"
        :
        : "g"(tf)
        : "memory");
    panic("iret failed"); /* mostly to placate the compiler */
}

void proc_run(struct Proc *p) {
    if (curproc && curproc->proc_status == PROC_RUNNING) {
        curproc->proc_status = PROC_RUNABLE;
    }
    curproc = p;
    curproc->proc_status = PROC_RUNNING;
    lcr3(PADDR(p->proc_pgdir));

    proc_pop_tf(&(p->proc_tf));
}

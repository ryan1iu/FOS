#include <inc/error.h>
#include <inc/fs.h>
#include <inc/kclock.h>
#include <inc/mmu.h>
#include <inc/pmap.h>
#include <inc/proc.h>
#include <inc/stdio.h>
#include <inc/string.h>
#include <inc/x86.h>

static struct Page *page_free_list;  // 用来维护空闲物理页的链表
struct Page *pages;  // 存放Page结构体数组的起始位置指针

size_t npages;              // 系统可用物理页的总个数
static size_t npages_base;  // 系统base memory中包含的物理页的个数
pde_t *kern_pgdir;          // 内核页目录的起始地址

// --------------------------------------------------------------
// 检测物理内存
// --------------------------------------------------------------

static int nvram_read(int r) {
    return mc146818_read(r) | (mc146818_read(r + 1) << 8);
}

static void detect_memory(void) {
    size_t basemem, extmem, ext16mem, totalmem;

    // 通过读取CMOS来检测系统可用物理内存
    basemem = nvram_read(NVRAM_BASELO);
    extmem = nvram_read(NVRAM_EXTLO);
    ext16mem = nvram_read(NVRAM_EXT16LO) * 64;

    if (ext16mem)
        totalmem = 16 * 1024 + ext16mem;
    else if (extmem)
        totalmem = 1 * 1024 + extmem;
    else
        totalmem = basemem;

    npages = totalmem / (PGSIZE / 1024);
    npages_base = basemem / (PGSIZE / 1024);
}

// --------------------------------------------------------------
// 设置内核地址空间映射
// --------------------------------------------------------------

static char *nextfree;  // 指向内核顶部空闲虚拟地址

// 在初始化pages之前用来申请物理地址空间
static void *boot_alloc(uint32_t n) {
    char *result;
    extern char end[];
    if (!nextfree) {
        nextfree = ROUNDUP((char *)end, PGSIZE);
    }

    // 返回当前可用空闲地址
    result = nextfree;

    // 更新nextfree
    nextfree += ROUNDUP(n, PGSIZE);

    // 如果nextfree超出了可用的地址大小，返回NULL
    if (nextfree > end + npages * PGSIZE) {
        return NULL;
    }
    return result;
}

void mem_init(void) {
    uint32_t cr0;
    size_t n;

    // 检测可用物理内存大小
    detect_memory();

    // 申请一个Page用来存放内核的页目录
    kern_pgdir = (pde_t *)boot_alloc(PGSIZE);
    memset(kern_pgdir, 0, PGSIZE);

    // 申请Page结构体数组空间
    pages = (struct Page *)boot_alloc(npages * sizeof(struct Page));
    memset(pages, 0, npages * sizeof(struct Page));

    // 申请Proc结构体数组空间
    procs = (struct Proc *)boot_alloc(NPROC * sizeof(struct Proc));
    memset(procs, 0, sizeof(struct Proc) * NPROC);

    // 申请File结构体数组空间
    file_meta_list = (struct File *)boot_alloc(NFILE * sizeof(struct File));
    memset(file_meta_list, 0, sizeof(struct File) * NFILE);

    void *disktemp = (void *)boot_alloc(PTSIZE);

    page_init();
    map_region(kern_pgdir, KTEMP, PTSIZE, PADDR(disktemp), PTE_W);

    map_region(kern_pgdir, KSTACKTOP - KSTKSIZE, KSTKSIZE, PADDR(bootstack),
               PTE_W);

    map_region(kern_pgdir, KERNBASE, ROUNDUP((0xffffffff - KERNBASE), PGSIZE),
               0, PTE_W);

    lcr3(PADDR(kern_pgdir));

    cr0 = rcr0();
    cr0 |= CR0_PE | CR0_PG | CR0_AM | CR0_WP | CR0_NE | CR0_MP;
    cr0 &= ~(CR0_TS | CR0_EM);
    lcr0(cr0);
}

// 初始化Page 结构体数组以及page_free_list
void page_init(void) {
    int i;
    size_t npages_kern = ((uint32_t)nextfree - KERNBASE) / PGSIZE;
    size_t npages_io = IOPHYSMEM / PGSIZE;
    for (i = npages - 1; i >= 0; i--) {
        if (i == 0 || (npages_io <= i && i <= npages_io + npages_kern)) {
            pages[i].ref = 1;
            pages[i].link = NULL;
        } else {
            pages[i].ref = 0;
            pages[i].link = page_free_list;
            page_free_list = &pages[i];
        }
    }
}

// 申请一个page,如果alloc_zero为1，则将此page清零
struct Page *ppage_alloc(int alloc_zero) {
    // 如果内存用尽就返回NULL
    if (page_free_list == NULL) {
        return NULL;
    }

    struct Page *result;
    char *page_va;

    result = page_free_list;
    page_free_list = page_free_list->link;

    // 将申请的page的link字段置为NULL,防止double-free
    result->link = NULL;

    // 获取result的虚拟地址
    page_va = page2kva(result);

    // cprintf("page_va:%x\n", page_va);
    if (alloc_zero) {
        memset(page_va, 0, PGSIZE);
    }

    return result;
}

// 释放一个page到page_free_list
void ppage_free(struct Page *pp) {
    struct Page *p_page_free;

    if (pp->ref != 0) {
        panic("pp->pp_ref is nonzero\n");
    }
    if (pp->link != NULL) {
        panic("pp->pp_link is not NULL\n");
    }

    pp->link = page_free_list;
    page_free_list = pp;
}

// 将page的引用计数减一，当引用计数为0时，释放这个page
void ppage_decref(struct Page *pp) {
    if (--pp->ref == 0) ppage_free(pp);
}

// 将从pa起始的size大小的空间映射到va起始的位置
static void map_region(pde_t *pgdir, uintptr_t va, size_t size, physaddr_t pa,
                       int perm) {
    pte_t *pte;
    uint32_t n = size / PGSIZE;
    for (int i = 0; i < n; i++) {
        pte = va2pte(pgdir, (void *)va, 1);

        // 因为将creat置为1，所以如果va2pte返回NULL说明内存不足
        if (pte == NULL) {
            panic("boot_map_region: out of memory\n");
        }

        // 设置pte
        *pte = pa | perm | PTE_P;
        va += PGSIZE;
        pa += PGSIZE;
    }
}

// 返回与va想对应的pte的指针
pte_t *va2pte(pde_t *pgdir, const void *va, int create) {
    pde_t *pde = pgdir + PDX(va);
    // 如果page table不存在
    if (!(*pde & PTE_P)) {
        if (!create) return NULL;
        // 申请一个page table
        struct Page *pp = ppage_alloc(1);
        if (pp == NULL) return NULL;
        pp->ref++;

        // 设置pde
        *pde = page2pa(pp) | PTE_P | PTE_U | PTE_W;
    }

    return (pte_t *)KADDR(PTE_ADDR(*pde)) + PTX(va);
}

// 将物理页pp映射到虚拟地址va
int vpage_insert(pde_t *pgdir, struct Page *pp, void *va, int perm) {
    physaddr_t ppa = page2pa(pp);
    pte_t *pte = va2pte(pgdir, va, 1);
    if (!pte) {
        return -E_NO_MEM;
    }
    if (*pte & PTE_P) {
        if (PTE_ADDR(*pte) == page2pa(pp)) {
            pp->ref--;
        } else {
            vpage_remove(pgdir, va);
        }
    }

    pp->ref++;
    *pte = page2pa(pp) | perm | PTE_P;

    return 0;
}

// 返回va所映射的page,如果没有返回NULL
struct Page *vpage_lookup(pde_t *pgdir, void *va, pte_t **pte_store) {
    pte_t *pte = va2pte(pgdir, va, 0);
    if (!pte || !(*pte & PTE_P)) {
        cprintf("page_lookup: can't find pte\n");
        return NULL;
    }
    if (pte_store) {
        *pte_store = pte;
    }
    return pa2page(PTE_ADDR(*pte));
}

// 取消将page映射到va
void vpage_remove(pde_t *pgdir, void *va) {
    pte_t *pte_store;
    struct Page *page = vpage_lookup(pgdir, va, &pte_store);

    if (!page) {
        cprintf("page_remove: no page related to va\n");
    } else {
        ppage_decref(page);
        *pte_store = 0;
        // tlb_invalidate(pgdir, va);
        invlpg(va);
    }
}

// Invalidate a TLB entry, but only if the page tables being
// edited are the ones currently in use by the processor.
// void tlb_invalidate(pde_t *pgdir, void *va) {
//     // Flush the entry only if we're modifying the current address space.
//     if (!curenv || curenv->env_pgdir == pgdir) invlpg(va);
// }

// static uintptr_t user_mem_check_addr;
// int user_mem_check(struct Env *env, const void *va, size_t len, int perm) {
//     if ((uintptr_t)va >= ULIM) {
//         // condition 1 - below ULIM violated
//         user_mem_check_addr = (uintptr_t)va;
//         return -E_FAULT;
//     }
//
//     uintptr_t va_start = (uintptr_t)ROUNDDOWN(va, PGSIZE);
//     uintptr_t va_end = (uintptr_t)ROUNDUP(va + len, PGSIZE);
//
//     for (; va_start < va_end; va_start += PGSIZE) {
//         // note we set page directory entry with less restrict
//         // we will only test page table entry here
//         pte_t *pgtable_entry_ptr =
//             pgdir_walk(env->env_pgdir, (char *)va_start, false);
//         if (pgtable_entry_ptr &&
//             ((*pgtable_entry_ptr & (perm | PTE_P)) == (perm | PTE_P))) {
//             continue;
//         }
//
//         // condition 2 - permission violated
//         if (va_start <= (uintptr_t)va) {
//             // va lie in the first page and not
//             // aligned, return va
//             user_mem_check_addr = (uintptr_t)va;
//         } else if (va_start >= (uintptr_t)va + len) {
//             // va lie in the last page and exceed va +
//             // len, return va + len
//             user_mem_check_addr = (uintptr_t)va + len;
//         } else {
//             // return corresponding page's initial
//             // address
//             user_mem_check_addr = va_start;
//         }
//
//         return -E_FAULT;
//     }
//
//     // pass user memory check
//     return 0;
// }
// void user_mem_assert(struct Env *env, const void *va, size_t len, int perm) {
//     if (user_mem_check(env, va, len, perm | PTE_U) < 0) {
//         cprintf(
//             "[%08x] user_mem_check assertion failure for "
//             "va %08x\n",
//             env->env_id, user_mem_check_addr);
//         env_destroy(env);  // may not return
//     }
// }

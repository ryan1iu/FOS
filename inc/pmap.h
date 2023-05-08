#ifndef INC_PMAP_H
#define INC_PMAP_H
#include <inc/assert.h>
#include <inc/memlayout.h>
#include <inc/types.h>

#include "inc/mmu.h"

// 用来保存物理页信息的结构体
struct Page {
    uint32_t ref;       // 该页被映射到虚拟地址空间的数量
    struct Page *link;  // 指向下一个空闲的Page
};

static struct Page *page_free_list;  // 用来维护空闲物理页的链表
extern pde_t *kern_pgdir;
extern struct Page *pages;
extern size_t npages;
static size_t npages_base;  // 系统base memory中包含的物理页的个数
                            //
extern char bootstacktop[],
    bootstack[];  // 内核启动栈的起始和结束地址，在entry.S中初始化

static void page_init();  // 初始化page_free_list

void mem_init(void);  // 初始化内核地址空间布局

struct Page *ppage_alloc(int);  // 申请一个空闲Page

void ppage_free(struct Page *);  // 释放一个Page
void ppage_decref(struct Page *pp);

pte_t *va2pte(pde_t *pgdir, const void *va, int create);

int vpage_insert(pde_t *pgdir, struct Page *pp, void *va, int perm);
void vpage_remove(pde_t *pgdir, void *va);

void tlb_invalidate(pde_t *pgdir, void *va);

static void map_region(pde_t *pgdir, uintptr_t va, size_t size, physaddr_t pa,
                       int perm);

#define PADDR(kva) _paddr(__FILE__, __LINE__, kva)

static inline physaddr_t _paddr(const char *file, int line, void *kva) {
    if ((uint32_t)kva < KERNBASE)
        _panic(file, line, "PADDR called with invalid kva %08lx", kva);
    return (physaddr_t)kva - KERNBASE;  // 将虚拟地址转化为物理地址
}

#define KADDR(pa) _kaddr(__FILE__, __LINE__, pa)

static inline void *_kaddr(const char *file, int line, physaddr_t pa) {
    if (PGNUM(pa) >= npages) {
        cprintf("PGNUM(pa):%d, npages:%d\n", PGNUM(pa), npages);
        _panic(file, line, "KADDR called with invalid pa %08lx", pa);
    }
    return (void *)(pa + KERNBASE);  // 将物理地址转化为虚拟地址
}

//

static inline physaddr_t page2pa(struct Page *pp) {
    return (pp - pages) << PGSHIFT;
}

static inline struct Page *pa2page(physaddr_t pa) {
    if (PGNUM(pa) >= npages) panic("pa2page called with invalid pa");
    return &pages[PGNUM(pa)];
}

static inline void *page2kva(struct Page *pp) { return KADDR(page2pa(pp)); }

struct Page *vpage_lookup(pde_t *pgdir, void *va, pte_t **pte_store);
#endif

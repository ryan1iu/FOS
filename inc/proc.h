#ifndef INC_PROC_H
#define INC_PROC_H

#include <inc/trap.h>
#include <inc/types.h>

typedef int32_t pid_t;
extern struct Proc *procs;    // 所有的进程
extern struct Proc *curproc;  // 当前占据CPU的进程
extern struct Segdesc gdt[];  // 全局描述符表

#define NPROC 1024  // 最多同时运行的进程个数
// 进程结构体定义
struct Proc {
    pid_t proc_id;
    pid_t proc_parent_id;
    unsigned proc_status;      // 进程状态
    struct Trapframe proc_tf;  // 进程上下文环境
    struct Proc *proc_link;
    pde_t *proc_pgdir;  // 进程页目录的内核虚拟地址
    void *proc_pfcall;
};

enum {
    PROC_FREE = 0,
    PROC_NOT_RUNABLE,
    PROC_RUNABLE,
    PROC_RUNNING,
    PROC_DYING
};

// 函数定义
void proc_init(void);
int proc_alloc(struct Proc **e, pid_t parent_id);
void proc_free(struct Proc *e);
void proc_create(uint8_t *binary);
void proc_destroy(struct Proc *e);
void proc_run(struct Proc *e);
void proc_pop_tf(struct Trapframe *tf);

#endif

#ifndef LIBS_LIB_H
#define LIBS_LIB_H

#include <inc/assert.h>
#include <inc/error.h>
#include <inc/memlayout.h>
#include <inc/pmap.h>
#include <inc/proc.h>
#include <inc/stdarg.h>
#include <inc/stdio.h>
#include <inc/string.h>
#include <inc/trap.h>
#include <inc/types.h>

pid_t this_pid;
// main user program
void umain(int argc, char **argv);
void gomain(int argc, char **arhv);

// gomain.c or entry.S
extern const volatile struct Proc uprocs[NPROC];
extern const volatile struct Page upages[];

enum {
    S_puts = 0,
    S_getc,
    S_getpid,
    S_destoryproc,
    S_setpfcall,
    S_mappage,
    S_umappage,
    S_setstatus,
    S_allocpage,
    S_fork,
    S_yield,
    S_unkown
};

void sys_puts(const char *string, size_t len);
int sys_getc(void);
pid_t sys_getpid(void);
int sys_destoryproc(pid_t);
void sys_yield();

pid_t fork();
// fork原语
int sys_mappage(pid_t srcenv, void *srcva, pid_t dstenv, void *dstva, int perm);
int sys_umappage(pid_t pid, void *va);
int sys_allocpage(pid_t pid, void *va, int perm);
int sys_setstatus(pid_t pid, int status);
int sys_setpfcall(pid_t pid, void *call);

static inline pid_t sys_fork(void) {
    pid_t ret;
    asm volatile("int %2" : "=a"(ret) : "a"(S_fork), "i"(T_SYSCALL));
    return ret;
}
void pf_set(void(*handler));

int getchar();
void putchar(int c);

#endif

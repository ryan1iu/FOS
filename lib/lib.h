#ifndef LIB_LIB_H
#define LIB_LIB_H

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

void umain(int argc, char **argv);
void gomain(int argc, char **arhv);

enum {
    S_puts = 0,
    S_getc,
    S_getpid,
    S_destoryproc,
    S_cowfork,
    S_exec,
    S_yield,
    S_getfid,
    S_unkown
};

void sys_puts(const char *string, size_t len);
int sys_getc(void);
pid_t sys_getpid(void);
fid_t sys_getfid(const char *filename);
int sys_destoryproc(pid_t);
void sys_yield();

pid_t fork();
int getchar();
void putchar(int c);

#endif

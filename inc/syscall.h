#ifndef INC_SYSCALL_H
#define INC_SYSCALL_H

#include <inc/types.h>

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
int32_t syscall(uint32_t num, uint32_t a1, uint32_t a2, uint32_t a3,
                uint32_t a4, uint32_t a5);

#endif

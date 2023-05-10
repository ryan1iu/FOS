#ifndef INC_SYSCALL_H
#define INC_SYSCALL_H

#include <inc/types.h>

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
int32_t syscall(uint32_t num, uint32_t a1, uint32_t a2, uint32_t a3,
                uint32_t a4, uint32_t a5);

#endif

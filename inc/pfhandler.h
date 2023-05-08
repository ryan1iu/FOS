#ifndef INC_PFHANDLER_H
#define INC_PFHANDLER_H

#include <inc/trap.h>

void handle_pgfault(struct Trapframe *tf);

#endif

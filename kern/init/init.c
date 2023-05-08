#include <inc/console.h>
#include <inc/monitor.h>
#include <inc/pmap.h>
#include <inc/proc.h>
#include <inc/sched.h>
#include <inc/stdio.h>
#include <inc/string.h>
#include <inc/trap.h>

void kern_init(void) {
    console_init();
    mem_init();
    proc_init();
    trap_init();
    // monitor();

    extern uint32_t _binary_obj_user_mpadd_start;
    proc_create((uint8_t *)&_binary_obj_user_mpadd_start);

    sched_yield();
}

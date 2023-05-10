#include <inc/console.h>
#include <inc/fs.h>
#include <inc/irq.h>
#include <inc/monitor.h>
#include <inc/pmap.h>
#include <inc/proc.h>
#include <inc/sched.h>
#include <inc/stdio.h>
#include <inc/string.h>
#include <inc/trap.h>

#include "inc/disk.h"
#include "inc/memlayout.h"

void kern_init(void) {
    console_init();
    plogo();
    mem_init();
    fs_init();
    proc_init();
    trap_init();
    pic_init();
    // extern uint32_t _binary_obj_user_spin_start;
    // proc_create((uint8_t *)&_binary_obj_user_spin_start);

    // extern uint32_t _binary_obj_user_testkd_start;
    // proc_create((uint8_t *)&_binary_obj_user_testkd_start);

    // extern uint32_t _binary_obj_user_hello_start;
    // proc_create((uint8_t *)&_binary_obj_user_hello_start);
    // proc_create((uint8_t *)&_binary_obj_user_hello_start);

    // extern uint32_t _binary_obj_user_mpadd_start;
    // proc_create((uint8_t *)&_binary_obj_user_mpadd_start);

    // extern uint32_t _binary_obj_user_fork_start;
    // proc_create((uint8_t *)&_binary_obj_user_fork_start);
    struct File *file = file_meta_list;
    file += 5;
    read_data(file->name, (char *)KTEMP, file->sector, file->size);

    proc_create((void *)KTEMP);

    sched_yield();
}

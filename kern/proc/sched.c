#include <inc/assert.h>
#include <inc/monitor.h>
#include <inc/pmap.h>
#include <inc/proc.h>
#include <inc/sched.h>
#include <inc/x86.h>

void sched_halt(void);

//
// 循环任务调度，如果没有可运行任务就跳转到sched_halt
//
void sched_yield(void) {
    int proc_index;
    if (curproc != NULL) {
        proc_index = curproc->proc_id - 1;
        for (int i = proc_index + 1; i < NPROC; i++) {
            if (procs[i].proc_status == PROC_RUNABLE) {
                proc_run(&procs[i]);
                return;
            }
        }
        for (int i = 0; i < proc_index; i++) {
            if (procs[i].proc_status == PROC_RUNABLE) {
                proc_run(&procs[i]);
                return;
            }
        }

        if (curproc->proc_status == PROC_RUNNING) {
            proc_run(curproc);
            return;
        }

        sched_halt();
    } else {
        for (int i = 0; i < NPROC; i++) {
            if (procs[i].proc_status == PROC_RUNABLE) {
                curproc = &procs[i];
                proc_run(&procs[i]);
                return;
            }
        }
        sched_halt();
    }
}

// Halt this CPU when there is nothing to do. Wait until the
// timer interrupt wakes it up. This function never returns.
//
void sched_halt(void) {
    int i;

    for (i = 0; i < NPROC; i++) {
        if ((procs[i].proc_status == PROC_RUNABLE ||
             procs[i].proc_status == PROC_RUNNING ||
             procs[i].proc_status == PROC_DYING))
            break;
    }
    if (i == NPROC) {
        while (1) monitor();
    }
}

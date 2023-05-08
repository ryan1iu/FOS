#include <lib/lib.h>

extern void _pf_call(void);
void (*_pf_handler)(struct UTrapframe *utf);

void pf_set(void(*handler)) {
    if (_pf_handler == NULL) {
        // 设置pgfault handler为统一的汇编入口
        if (sys_setpfcall(this_pid, _pf_call) < 0) {
            cprintf("set pgfault_upcall failed");
        }
    }

    // 真正的处理函数，汇编文件在进行一些处理之后跳转到此handler
    _pf_handler = handler;
}

#include <lib/lib.h>

extern void umain(int argc, char **argv);

void gomain(int argc, char **argv) {
    this_pid = sys_getpid();
    umain(argc, argv);
    sys_destoryproc(this_pid);
}

#include <lib/lib.h>

void umain(int argc, char **argv) {
    cprintf("my fid is %d\n", sys_getfid("getfid"));
}

#include <inc/disk.h>
#include <inc/error.h>
#include <inc/fs.h>
#include <inc/stdio.h>
#include <inc/string.h>
#include <inc/types.h>

uint32_t file_num = 0;
struct File* file_meta_list;

int read_data(char* filename, char* buffer, uint32_t sector, uint32_t size) {
    struct File* file_meta = NULL;
    int flag = 0;
    for (int i = 0; i < file_num; i++) {
        file_meta = &file_meta_list[i];
        if (strcmp(file_meta->name, filename) == 0) {
            flag = 1;
            break;
        }
    }
    if (flag == 0) {
        return -E_FILE_NF;
    }

    ide_read(sector, (void*)buffer, size);
    return 0;
}

void fs_init() {
    // 目的是加载元数据到内存中来
    ide_read(2, (void*)file_meta_list, 4);
    struct File* pfile;
    pfile = file_meta_list;
    // cprintf("pfilemagic : %x\n", pfile->magic);
    // for (int i = 0; i < 5; i++) {
    //     cprintf("%s\n", pfile->name);
    //     cprintf("%d\n", pfile->sector);
    //     cprintf("%d\n", pfile->size);
    //     pfile++;
    // }
    //
    while (1) {
        if (pfile->magic == 0xAA) {
            cprintf("load file %s\n", pfile->name);
            pfile++;
            file_num++;
        } else {
            break;
        }
    }
    cprintf("load %d files\n", file_num);
    // cprintf("pfilemagic : %x\n", pfile->magic);
}

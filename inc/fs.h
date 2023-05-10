#ifndef FS_FS_H
#define FS_FS_H
#include <inc/types.h>

#define NFILE 128
struct File {
    uint8_t magic;
    char name[64];
    uint32_t sector;
    uint32_t size;  // sector的数量
} __attribute__((packed));

extern struct File* file_meta_list;

void fs_init();

#endif

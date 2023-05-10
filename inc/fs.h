#ifndef FS_FS_H
#define FS_FS_H
#include <inc/types.h>

#define NFILE 128
struct File {
    uint8_t magic;
    char name[64];
    uint8_t fid;
    uint32_t sector;
    uint32_t size;  // sector的数量
} __attribute__((packed));

extern struct File* file_meta_list;
int read_data(char* filename, char* buffer, uint32_t sector, uint32_t size);
void fs_init();

fid_t fs_getfid(const char* filename);

#endif

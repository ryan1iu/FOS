#ifndef INC_DISK_H
#define INC_DISK_H

#include <inc/types.h>

#define IDE_BSY 0x80
#define IDE_DRDY 0x40
#define IDE_DF 0x20
#define IDE_ERR 0x01
#define SECTSIZE 512

int ide_read(uint32_t secno, void *dst, size_t nsecs);

#endif

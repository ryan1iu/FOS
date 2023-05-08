#include <inc/x86.h>
// xxx
#include <inc/elf.h>

#define SECTSIZE 512
#define ELFHDR ((struct Elf *)0x10000) // 内核的加载地址

void readsect(void *, uint32_t);
void readseg(uint32_t, uint32_t, uint32_t);

void bootmain(void) {
  struct Proghdr *ph, *eph;

  // 读取第一个扇区
  readseg((uint32_t)ELFHDR, SECTSIZE * 8, 0);

  // 判断是否为合法的ELF文件
  if (ELFHDR->e_magic != ELF_MAGIC)
    goto bad;

  // 循环加载每一个段
  ph = (struct Proghdr *)((uint8_t *)ELFHDR + ELFHDR->e_phoff);
  eph = ph + ELFHDR->e_phnum;
  for (; ph < eph; ph++)
    // ph->p_pa 是加载地址
    readseg(ph->p_pa, ph->p_memsz, ph->p_offset);

  // 跳转到内核入口
  ((void (*)(void))(ELFHDR->e_entry))();

bad:
  outw(0x8A00, 0x8A00);
  outw(0x8A00, 0x8E00);
  while (1)
    /* do nothing */;
}

void readseg(uint32_t pa, uint32_t count, uint32_t offset) {
  uint32_t end_pa;

  end_pa = pa + count;

  // round down to sector boundary
  pa &= ~(SECTSIZE - 1);

  // translate from bytes to sectors, and kernel starts at sector 1
  offset = (offset / SECTSIZE) + 1;

  while (pa < end_pa) {
    readsect((uint8_t *)pa, offset);
    pa += SECTSIZE;
    offset++;
  }
}

void waitdisk(void) {
  // wait for disk reaady
  while ((inb(0x1F7) & 0xC0) != 0x40)
    /* do nothing */;
}

void readsect(void *dst, uint32_t offset) {
  // wait for disk to be ready
  waitdisk();

  outb(0x1F2, 1); // count = 1
  outb(0x1F3, offset);
  outb(0x1F4, offset >> 8);
  outb(0x1F5, offset >> 16);
  outb(0x1F6, (offset >> 24) | 0xE0);
  outb(0x1F7, 0x20); // cmd 0x20 - read sectors

  // wait for disk to be ready
  waitdisk();
  // read a sector
  insl(0x1F0, dst, SECTSIZE / 4);
}

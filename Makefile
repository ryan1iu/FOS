GDBPORT := 26000
TOP = .

CC := i386-fos-elf-gcc -pipe
GDB := gdb 
AS := as
AR := ar
LD := ld

CFLAGS := -O1 -fno-builtin -fno-tree-ch -I$(TOP) -MD
CFLAGS += -fno-omit-frame-pointer
CFLAGS += -std=gnu99
CFLAGS += -static
CFLAGS += -gstabs 
CFLAGS += -Wall -Wno-format -Wno-unused -Werror -m32
CFLAGS += -fno-stack-protector

LD_FLAGS := -m elf_i386
ULDFLAGS := -T user/user.ld

KERN_CFLAGS := $(CFLAGS) -DKERNEL
USER_CFLAGS := $(CFLAGS) -DUSER

GCC_LIB := /usr/local/lib/gcc/i386-fos-elf/4.6.4/libgcc.a

include kern/kern.mk
include lib/lib.mk
include user/user.mk
include boot/boot.mk

QEMU := /usr/local/bin/qemu-system-i386
QEMUOPTS = -drive file=obj/kern/kernel.img,index=0,media=disk,format=raw -m 128 -serial mon:stdio -gdb tcp::$(GDBPORT)

gdb:
	gdb -n -tui -x .gdbinit

qemu: obj/kern/kernel.img
	@$(QEMU) $(QEMUOPTS) 2>/dev/null

qemu-nox: obj/kern/kernel.img
	@$(QEMU) -nographic $(QEMUOPTS)

qemu-gdb: obj/kern/kernel.img
	@$(QEMU) $(QEMUOPTS) -S 2>/dev/null

qemu-nox-gdb: obj/kern/kernel.img
	@$(QEMU) -nographic $(QEMUOPTS) -S

clean:
	rm -f obj/kern/kernel.img

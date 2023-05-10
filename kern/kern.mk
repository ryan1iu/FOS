KERN_LDFLAGS := $(LD_FLAGS) -T kern/kern.ld -nostdlib

KERN_OBJFILES := obj/kern/monitor.o \
				 obj/kern/panic.o \
				 obj/kern/printf.o \
				 obj/kern/readline.o \
				 obj/kern/console.o \
				 obj/kern/disk.o \
				 obj/kern/kclock.o \
				 obj/kern/keyboard.o \
				 obj/kern/fs.o \
				 obj/kern/entry.o \
				 obj/kern/entrypgdir.o \
				 obj/kern/init.o \
				 obj/kern/mem.o \
				 obj/kern/proc.o \
				 obj/kern/sched.o \
				 obj/kern/irq.o \
				 obj/kern/pfhandler.o \
				 obj/kern/syscall.o \
				 obj/kern/trap.o \
				 obj/kern/trapall.o 

# 库文件
KERN_OBJFILES := $(KERN_OBJFILES) \
				 obj/kern/printfmt.o \
				 obj/kern/string.o \

# 嵌入到内核中的用户可执行文件
KERN_USERFILES := obj/user/hello \
				  obj/user/fork \
				  obj/user/mpadd \
				  obj/user/spin \
				  obj/user/testkd \
				  obj/user/getfid
					

obj/kern/kernel.img: obj/kern/kernel obj/boot/boot obj/user/ obj/disk/user_disk.img
	dd if=/dev/zero of=obj/kern/kernel.img~ count=10000
	dd if=obj/boot/boot of=obj/kern/kernel.img~ conv=notrunc
	dd if=obj/kern/kernel of=obj/kern/kernel.img~ seek=1 conv=notrunc
	mv obj/kern/kernel.img~ obj/kern/kernel.img

obj/kern/%.o: kern/debug/%.c
	$(CC) $(KERN_CFLAGS) -c -o $@ $<

obj/kern/%.o: kern/driver/%.c
	$(CC) $(KERN_CFLAGS) -c -o $@ $<

obj/kern/%.o: kern/fs/%.c
	$(CC) $(KERN_CFLAGS) -c -o $@ $<

obj/kern/%.o: kern/init/%.c
	$(CC) $(KERN_CFLAGS) -c -o $@ $<

obj/kern/%.o: kern/init/%.S
	$(CC) $(KERN_CFLAGS) -c -o $@ $<

obj/kern/%.o: kern/mem/%.c
	$(CC) $(KERN_CFLAGS) -c -o $@ $<

obj/kern/%.o: kern/proc/%.c
	$(CC) $(KERN_CFLAGS) -c -o $@ $<

obj/kern/%.o: kern/trap/%.c
	$(CC) $(KERN_CFLAGS) -c -o $@ $<

obj/kern/%.o: kern/trap/%.S
	$(CC) $(KERN_CFLAGS) -c -o $@ $<

obj/kern/%.o: lib/%.c
	$(CC) $(KERN_CFLAGS) -c -o $@ $<

obj/kern/kernel: $(KERN_OBJFILES) $(KERN_USERFILES) kern/kern.ld
	$(LD) -o $@ $(KERN_LDFLAGS) $(KERN_OBJFILES) $(GCC_LIB) -b binary $(KERN_USERFILES)  

obj/disk/user_disk.img: obj/user
	tools/wdisk
	dd if=obj/disk/user_disk of=obj/disk/user_disk.img seek=1 conv=notrunc





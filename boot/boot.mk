BOOT_OBJS := obj/boot/boot.o obj/boot/main.o

obj/boot/boot: $(BOOT_OBJS)
	$(LD) $(LD_FLAGS) -N -e start -Ttext 0x7c00 -o $@.out $^
	objcopy -S -O binary -j .text $@.out $@
	perl boot/sign.pl obj/boot/boot

obj/boot/main.o: boot/main.c
	$(CC) -nostdinc $(KERN_CFLAGS) -Os -c -o $@ $<

obj/boot/boot.o: boot/boot.S
	$(CC) -nostdinc $(KERN_CFLAGS) -c -o $@ $<



obj/user/%.o: user/%.c 
	$(CC) -nostdinc $(USER_CFLAGS) -c -o $@ $<

obj/user/%: obj/user/%.o obj/lib/entry.o obj/lib/libfos.a user/user.ld
	$(LD) -o $@ $(ULDFLAGS) $(LDFLAGS) -nostdlib obj/lib/entry.o $@.o -Lobj/lib -lfos $(GCC_LIB)


LIB_OBJFILES := obj/lib/console.o \
				obj/lib/entry.o \
				obj/lib/fork.o \
				obj/lib/gomain.o \
				obj/lib/pagefault.o \
				obj/lib/pfentry.o \
				obj/lib/printfmt.o \
				obj/lib/readline.o \
				obj/lib/string.o \
				obj/lib/syscall.o \
				obj/lib/uprintf.o

obj/lib/libfos.a: $(LIB_OBJFILES)
	$(AR) r $@ $(LIB_OBJFILES)

obj/lib/%.o: lib/%.c 
	$(CC) -nostdinc $(USER_CFLAGS) -c -o $@ $<

obj/lib/%.o: lib/%.S 
	$(CC) -nostdinc $(USER_CFLAGS) -c -o $@ $<



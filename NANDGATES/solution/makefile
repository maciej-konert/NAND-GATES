CC 		 = gcc
CPPFLAGS =
CFLAGS   = -Wall -Wextra -Wno-implicit-fallthrough -std=gnu17 -fPIC -O2
LDFLAGS  = -shared -Wl,--wrap=malloc -Wl,--wrap=calloc -Wl,--wrap=realloc -Wl,--wrap=reallocarray -Wl,--wrap=free -Wl,--wrap=strdup -Wl,--wrap=strndup

.PHONY: all clean example testy

export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/home/students/inf/m/mk459179/PRZEDMIOTY/AKSO/PROJEKTY/Projekt1v2

memory_tests.o: memory_tests.c memory_tests.h
	$(CC) $(CFLAGS) -c -o $@ $<
nand.o: nand.c nand.h
	$(CC) $(CFLAGS) -c -o $@ $<
nand_example.o: nand_example.c nand.h memory_tests.h
	$(CC) $(CFLAGS) -c -o $@ $<
testy.o: testy.c nand.h
	$(CC) $(CFLAGS) -c -o $@ $<

all: libnand.so

libnand.so: nand.o memory_tests.o
	$(CC) $(LDFLAGS) -o $@ $^

example: nand_example.o libnand.so
	gcc -L. -Wl,-rpath=$(PWD) $< -lnand -o $@

testy: testy.o libnand.so
	gcc -L. -Wl,-rpath=$(PWD) $< -lnand -o $@
clean:
	rm -f *.o libnand.so example testy

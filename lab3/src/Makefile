CC=g++
CXXFLAGS=-g -std=c++11
CFLAGS=-I. -g
DEPS=mmu.hpp

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

linker: Process.o mmu.o utils.o Pager.o
	$(CC) $(CFLAGS) -o mmu Process.o mmu.o utils.o Pager.o

clean:
	rm -rf mmu *.o
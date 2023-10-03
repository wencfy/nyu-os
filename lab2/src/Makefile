CC=g++
CXXFLAGS=-g -std=c++11
CFLAGS=-I. -g
DEPS = scheduler.hpp

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

linker: sched.o scheduler.o
	$(CC) $(CFLAGS) -o sched scheduler.o sched.o

clean:
	rm -rf sched *.o
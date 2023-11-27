CC=g++
CXXFLAGS=-g -std=c++11
CFLAGS=-I. -g
DEPS = iosched.hpp

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

linker: iosched.o Loader.o Scheduler.o
	$(CC) $(CFLAGS) -o iosched iosched.o Loader.o Scheduler.o

clean:
	rm -rf iosched *.o
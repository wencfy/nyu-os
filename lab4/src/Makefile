CC=g++
CXXFLAGS=-g -std=c++11
CFLAGS=-I. -g
# DEPS = scheduler.hpp

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

linker: iosched.o 
	$(CC) $(CFLAGS) -o iosched iosched.o

clean:
	rm -rf iosched *.o
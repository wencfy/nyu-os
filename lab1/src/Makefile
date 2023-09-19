CC=g++
CXXFLAGS=-g -std=c++11
CFLAGS=-I. -g
DEPS = helper.hpp

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

# tokenizer: tokenizer.o helper.o
# 	$(CC) $(CFLAGS) -o tokenizer helper.o tokenizer.o

linker: linker.o helper.o
	$(CC) $(CFLAGS) -o linker helper.o linker.o

clean:
	rm -rf tokenizer linker *.o

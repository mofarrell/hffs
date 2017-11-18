# Author: Michael O'Farrell
# Build for hfs file recovery.

CC=clang++
CFLAGS=-std=c++11 -stdlib=libc++
PROG=hffs
OBJS=hffs.o


all: $(PROG)

$(PROG): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $@


hffs.o:

main: main.cpp hfs/hfs_format.h
	clang++ -g -w -std=c++11 -stdlib=libc++ $< -o $@

.PHONY: clean
clean:
	rm -rf *~ *.o *.dSYM $(PROG) main a.out
	

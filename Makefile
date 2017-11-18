# Author: Michael O'Farrell
# Build for hfs file recovery.

CXX=clang++
CXXFLAGS=-std=c++11
PROG=hffs
OBJS=hffs.o recover.o

all: $(PROG)

$(PROG): $(OBJS)
	$(CXX) $(CFLAGS) $(OBJS) -o $@

hffs.o: rgs.h recover.h
recover.o: convert.h hfs/hfs_format.h recover.h rgs.h

main: main.cpp hfs/hfs_format.h
	clang++ -g -w -std=c++11 -stdlib=libc++ $< -o $@

.PHONY: clean
clean:
	rm -rf *~ *.o *.dSYM $(PROG) main a.out
	

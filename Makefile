# Author: Michael O'Farrell
# Build for hfs file recovery.

CXX=clang++
CXXFLAGS=-std=c++11
PROG=hffs
OBJS=hffs.o recover.o

all: $(PROG)

$(PROG): $(OBJS)
	$(CXX) $(CFLAGS) $(OBJS) -o $@

RGS_INCLUDES=rgs.h hfs/hfs_format.h hfs/hfs_unistr.h

hffs.o: $(RGS_INCLUDES) recover.h
recover.o: $(RGS_INCLUDES) hfs/hfs_format.h hfs/hfs_unistr.h convert.h \
													 recover.h

main: main.cpp hfs/hfs_format.h
	clang++ -g -w -std=c++11 -stdlib=libc++ $< -o $@

.PHONY: clean
clean:
	rm -rf *~ *.o *.dSYM $(PROG) main a.out
	

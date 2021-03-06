# Author: Michael O'Farrell
# Build for hfs file recovery.

UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Darwin)
	CXX=clang++
else
	CXX=g++
endif

CXXFLAGS=-std=c++11 -g
PROG=hffs
OBJS=hffs.o recover.o

all: $(PROG)

$(PROG): $(OBJS)
	$(CXX) $(CFLAGS) $(OBJS) -o $@

RGS_INCLUDES=rgs.h hfs/hfs_format.h hfs/hfs_unistr.h

hffs.o: $(RGS_INCLUDES) recover.h
recover.o: $(RGS_INCLUDES) hfs/hfs_format.h hfs/hfs_unistr.h convert.h \
													 recover.h

.PHONY: clean
clean:
	rm -rf *~ *.o *.dSYM $(PROG)
	

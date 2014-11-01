######################################################################
#  Makefile for anpa-1312
######################################################################

PREFIX	= /usr/local

DEFNS	= 
STD	= -std=gnu++0x

GCOPTS	= -Wall -g -O0
GLIBS	= -lstdc++ -lc -lm 

INCL	= -I.

CPPFLAGS = $(GOPTS) $(INCL) $(OPTIONS) $(INCL) $(DEFNS)
CXXFLAGS = $(STD) $(GCOPTS) $(CPPFLAGS)

CXX	= $(ARCH)c++
AR	= $(ARCH)ar
MAKE	= gmake

.cpp.o:
	$(CXX) $(CXXFLAGS) -c $< -o $*.o

CPP_FILES 	:= $(wildcard *.cpp)
OBJS 		:= $(C_FILES:.c=.o) $(CPP_FILES:.cpp=.o)

all:	anpa1312

OBJS	= anpa1312.o

anpa1312: $(OBJS)
	$(CXX) $(OBJS) -o anpa1312 $(GLIBS)

install: anpa1312
#	install -m 444 libsqlite3db.a $(PREFIX)/lib 

uninstall:
#	rm -f $(PREFIX)/lib/libsqlite3db.a $(PREFIX)/include/sqlite3db.hpp

clean:
	rm -f *.o anpa1312

clobber: clean
	rm -f anpa1312 errs.t

######################################################################
#  End Makefile
######################################################################

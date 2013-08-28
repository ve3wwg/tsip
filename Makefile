######################################################################
#  Trimble & TSIP C++ Routines
######################################################################

OPTZ	= -g -O0
INCL	= -I/opt/local/include
OPTS	= -Wall $(INCL)
CFLAGS	= $(OPTZ) $(OPTS) 
CXXFLAGS= $(OPTZ) $(OPTS) -std=c++0x
LDFLAGS	= -L/opt/local/lib
CC	= gcc
CXX	= g++

.cpp.o:
	$(CXX) -c $(CXXFLAGS) $< -o $*.o

trimble: trimble.o ttyio.o tsip.o
	$(CXX) trimble.o ttyio.o tsip.o -o trimble $(LDFLAGS)

clean:
	rm -f *.o core* errs.t rpkt.dat rtest.dat

clobber: clean
	rm -f a.out test tsip.dat rstruct.h decode.c rgen rchk rgen.c rchk.c

tsip.o:	tsip.hpp

# End

OPTZ	= -g -O0
INCL	= -I/opt/local/include
OPTS	= -Wall $(INCL)
CFLAGS	= $(OPTZ) $(OPTS) 
CXXFLAGS= $(OPTZ) $(OPTS) -std=c++0x
LDFLAGS	= -L/opt/local/lib
CC	= gcc
CXX	= g++

# .c.o:
#	$(CC) -c $(CFLAGS) $< -o $*.o

.cpp.o:
	$(CXX) -c $(CXXFLAGS) $< -o $*.o

trimble: trimble.o ttyio.o tsip.o
	$(CXX) trimble.o ttyio.o tsip.o -o trimble $(LDFLAGS)

was:	test rgen rchk rtest

test:	test.o tsip.o decode.c
	$(CC) test.o tsip.o -o ./test 

rgen:	rgen.o tsip.o
	$(CC) rgen.o tsip.o -o rgen

rchk:	rchk.o tsip.o
	$(CC) rchk.o tsip.o -o rchk

rtest:	rgen rchk
	./rgen
	./rchk

clean:
	rm -f *.o core* errs.t rpkt.dat rtest.dat

clobber: clean
	rm -f a.out test tsip.dat rstruct.h decode.c rgen rchk rgen.c rchk.c

tsip.o:	tsip.hpp

# decode.c: msgs.dat gen.awk
# 	awk -f gen.awk msgs.dat

# rgen.c: decode.c
# rchk.c: decode.c

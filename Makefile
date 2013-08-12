OPTZ	= -g -O0
OPTS	= -Wall
CFLAGS	= $(OPTZ) $(OPTS)
CC	= gcc

.c.o:
	$(CC) -c $(CFLAGS) $< -o $*.o

all:	test rgen rchk rtest

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

tsip.o:	tsip.h
test.o:	tsip.h

decode.c: msgs.dat gen.awk
	awk -f gen.awk msgs.dat

rgen.c: decode.c
rchk.c: decode.c

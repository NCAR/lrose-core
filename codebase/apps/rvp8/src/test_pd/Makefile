# Makefile for lirp server and client
#

CC = gcc
MACH_OS = LINUX

#DEBUGFLAG = -O2
#DEBUGFLAG = -g -pg
DEBUGFLAG = -g

INCLUDES = -I./ -I/usr/sigmet/include -I../include

CFLAGS		= ${INCLUDES} ${DEBUGFLAG} -D${MACH_OS} -DSYSV

LDFLAGS         = -L. -L/usr/sigmet/rda/lib -L/usr/sigmet/libs/lib 

all: pd_single pd_dual

pd_single: pd_single.o
	${CC} ${LD_OPT} pd_single.o ${LDFLAGS} -lrdasubs -ldsp -ldspi \
	-lantenna -lconfig -luser -lm -lz -o pd_single

pd_dual: pd_dual.o
	${CC} ${LD_OPT} pd_dual.o ${LDFLAGS} -lrdasubs -ldsp -ldspi \
	-lantenna -lconfig -luser -lm -lz -o pd_dual

pd_single.o: pd_single.c
	${CC} ${CFLAGS} -c pd_single.c

pd_dual.o: pd_dual.c
	${CC} ${CFLAGS} -c pd_dual.c

clean_all:
	rm -f pd_single pd_dual core

clean:	clean_all
	rm -f *.o *.usg *.cat* *~



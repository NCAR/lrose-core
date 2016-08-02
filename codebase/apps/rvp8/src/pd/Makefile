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

all: pd

pd: pd.o
	${CC} ${LD_OPT} pd.o ${LDFLAGS} -lrdasubs -ldsp -ldspi \
	-lantenna -lconfig -luser -lm -lz -o pd

pd.o: pd.c
	${CC} ${CFLAGS} -c pd.c

clean_all:
	rm -f pd core

clean:	clean_all
	rm -f *.o *.usg *.cat* *~



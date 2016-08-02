# Makefile for lirp server and client
#
DEBUGFLAG = -g -pg
#DEBUGFLAG = -g

CC = gcc
MACH_OS = LINUX
DEBUGFLAG = -O2

CFLAGS_ALL	= -D${MACH_OS} -DMOTIF -DSYSV -DXT_CODE -DXOPEN_CATALOG \
				  -I./ -I/usr/sigmet/include -I./include
#CFLAGS_ALL	= -D${MACH_OS} -DMOTIF -DSYSV -DXT_CODE -DXOPEN_CATALOG \
#				  -DTESTING -I./ -I/usr/sigmet/include -I./include

#CFLAGS		= -c ${C_OPT}  ${CFLAGS_ALL}
CFLAGS		= -c ${DEBUGFLAG} ${C_OPT}  ${CFLAGS_ALL}
CXXFLAGS        = -c ${DEBUGFLAG} ${CC_OPT} ${CFLAGS_ALL}
CCFLAGS = ${CXXFLAGS}

CFLAGS_RELAXED	= -c ${DEBUGFLAG} ${C_OPT_RELAXED}  ${CFLAGS_ALL}
CCFLAGS_RELAXED = -c ${DEBUGFLAG} ${CC_OPT_RELAXED} ${CFLAGS_ALL}

#LDFLAGS         = -static -L. -L/usr/sigmet/rda/lib -L/usr/sigmet/libs/lib 
LDFLAGS         = -L. -L/usr/sigmet/rda/lib -L/usr/sigmet/libs/lib 

all:	lirpServer lirpClient 
.PHONY:	all

lirpServer:	svrMain.o sock_io.o
		${CXX} ${LD_OPT} svrMain.o sock_io.o ${LDFLAGS} -lrdasubs -ldspi \
		-lantenna -lconfig -luser -lm ${LSEARCH_BSD} -o lirpServer 
#lirpClient:	cliMain.o sock_io.o
#		${CXX} ${LD_OPT} cliMain.o sock_io.o ${LDFLAGS} -lrdasubs -ldspi \
#		-lantenna -lconfig -luser -lm ${LSEARCH_BSD} -o lirpClient 
lirpClient:	cliMain.o sock_io.o
		${CXX} ${LD_OPT} cliMain.o sock_io.o ${LDFLAGS} \
		-lm ${LSEARCH_BSD} -o lirpClient 

svrMain.o : ./include/lirpCommon.h ./include/lirpErrors.h
cliMain.o : ./include/lirpClient.h ./include/lirpCommon.h \
            ./include/lirpErrors.h
sock_io.o : ./include/lirpClient.h ./include/lirpCommon.h \
            ./include/lirpErrors.h

MKDEPEND_CSRCS = 

depend:; ${MKDEPEND}

cleanexe:;	rm -f ${EXES} core
clean:		cleanexe
		rm -f *.o *.usg *.cat*


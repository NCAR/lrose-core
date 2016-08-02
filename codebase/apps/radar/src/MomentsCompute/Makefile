# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
# ** Copyright UCAR (c) 1990 - 2016                                         
# ** University Corporation for Atmospheric Research (UCAR)                 
# ** National Center for Atmospheric Research (NCAR)                        
# ** Boulder, Colorado, USA                                                 
# ** BSD licence applies - redistribution and use in source and binary      
# ** forms, with or without modification, are permitted provided that       
# ** the following conditions are met:                                      
# ** 1) If the software is modified to produce derivative works,            
# ** such modified software should be clearly marked, so as not             
# ** to confuse it with the version available from UCAR.                    
# ** 2) Redistributions of source code must retain the above copyright      
# ** notice, this list of conditions and the following disclaimer.          
# ** 3) Redistributions in binary form must reproduce the above copyright   
# ** notice, this list of conditions and the following disclaimer in the    
# ** documentation and/or other materials provided with the distribution.   
# ** 4) Neither the name of UCAR nor the names of its contributors,         
# ** if any, may be used to endorse or promote products derived from        
# ** this software without specific prior written permission.               
# ** DISCLAIMER: THIS SOFTWARE IS PROVIDED "AS IS" AND WITHOUT ANY EXPRESS  
# ** OR IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED      
# ** WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.    
# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
###########################################################################
#
# Makefile for MomentsCompute program
#
# Mike Dixon, RAP, NCAR
# POBox 3000, Boulder, CO, 80307, USA
#
# Dec 2006
#
###########################################################################

TARGET_FILE = MomentsCompute

SHELL = /bin/sh
RM = /bin/rm -f
MV = /bin/mv -f
CP = /bin/cp
CP_DIR = /bin/cp -r
MKDIR = /bin/mkdir -p
TEST = /usr/bin/test
PERL = local/bin/perl
AWK = /usr/bin/awk

CC = g++
# CC = pgCC
MACH_OS = LINUX
INCLUDES = -I/usr/sigmet/include -I$(RVP8_INC_DIR)
DEBUG_FLAGS = -g
OPT_FLAGS = -O4
LDFLAGS = -L/usr/sigmet/rda/lib -L/usr/sigmet/libs/lib -L$(RVP8_LIB_DIR)
LIBS = -lradar -lRadx -lFmq -lrapformats -ldsserver -ldidss -ltoolsa -lz -ldataport -lfftw3

SRCS = \
	Args.cc \
	Beam.cc \
	Fft.cc \
	Fields.cc \
	Main.cc \
	Moments.cc \
	MomentsMgr.cc \
	Params.cc \
	Pulse.cc \
	MomentsCompute.cc \
	umalloc.cc

OBJS = $(SRCS:.cc=.o)

all:
	$(MAKE) CFLAGS="$(DEBUG_FLAGS)" $(TARGET_FILE)

opt:
	$(MAKE) CFLAGS="$(OPT_FLAGS)" $(TARGET_FILE)

static:
	$(MAKE) CFLAGS="$(DEBUG_FLAGS) -static" $(TARGET_FILE)

staticopt:
	$(MAKE) CFLAGS="$(OPT_FLAGS) -static" $(TARGET_FILE)

$(TARGET_FILE): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET_FILE)  $(OBJS) $(LDFLAGS) $(LIBS)

clean:
	$(RM) core a.out *.o *~ junk* *.ti *.ii

clean_bin:
	$(RM) $(TARGET_FILE)

clean_all: clean_bin clean

.SUFFIXES: .cc

.cc.o:
	$(CC) $(CFLAGS) -c $(INCLUDES) -D${MACH_OS} -DSYSV $<

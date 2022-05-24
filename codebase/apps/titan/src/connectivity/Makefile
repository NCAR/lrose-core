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
# Makefile for flowlength program
#
# Mike Dixon
#
# July 2004
#
###########################################################################

TARGET_FILE = flowlength
RM = /bin/rm -f
INCLUDES =
CFLAGS = -g -Wall
LDFLAGS =
C++ = g++

SRCS = \
	flowlengthslope.random.cc

OBJS = $(SRCS:.cc=.o)

all: $(TARGET_FILE)

# link

$(TARGET_FILE): $(OBJS) Makefile
	$(RM) $(TARGET_FILE)
	$(C++) $(CFLAGS) -o $(TARGET_FILE) $(OBJS) $(LDFLAGS)

# optimized

opt:
	$(MAKE) CFLAGS=-O2 all

# static link

static: $(OBJS) Makefile
	$(RM) $(TARGET_FILE)
	$(C++) $(CFLAGS) -static -o $(TARGET_FILE) $(OBJS) $(LDFLAGS)

# static link optimized

staticopt:
	$(MAKE) CFLAGS=-O2 static

# clean

clean:
	$(RM) *.o

clean_bin:
	$(RM) $(TARGET_FILE)

clean_all: clean clean_bin

# suffixes

.SUFFIXES: .cc

.cc.o:
	$(C++) $(CFLAGS) -c $(INCLUDES) $<

#
# local targets
#

# DO NOT DELETE THIS LINE -- make depend depends on it.


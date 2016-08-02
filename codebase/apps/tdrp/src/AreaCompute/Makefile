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
# Makefile for AreaCompute program
#
# Mike Dixon, RAP, NCAR, Boulder, CO, 80307, USA
#
# Sept 1998
#
###########################################################################

PROGNAME = AreaCompute

CPPC = g++
RM = /bin/rm -f
INCLUDES = -I$(RAP_INC_DIR)
CFLAGS = -g
LDFLAGS = -L$(RAP_LIB_DIR) -ltdrp

SRCS = \
	Params.cc \
	AreaCompute.cc \
	Args.cc \
	Main.cc

OBJS = $(SRCS:.cc=.o)

# link

$(PROGNAME): $(OBJS)
	$(CPPC) -o $(PROGNAME) $(OBJS) $(LDFLAGS)

# tdrp

Params.cc: paramdef.$(PROGNAME)
	tdrp_gen -f paramdef.$(PROGNAME) -c++ -prog $(PROGNAME)

clean_tdrp:
	$(RM) Params.hh Params.cc

clean:
	$(RM) core a.out
	$(RM) *.i *.o  *.ln *~

clean_bin:
	$(RM) $(PROGNAME)

clean_all: clean clean_bin

# suffix rules

.SUFFIXES: .c .o

.cc.o:
	$(CPPC) -c $(CFLAGS) $(INCLUDES) $<


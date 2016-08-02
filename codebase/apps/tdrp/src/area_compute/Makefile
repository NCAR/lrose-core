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
# Makefile for area_compute program
#
# Mike Dixon, RAP, NCAR, Boulder, CO, 80307, USA
#
# Sept 1998
#
###########################################################################

PROGNAME = area_compute

CC = gcc
RM = /bin/rm -f
INCLUDES = -I$(RAP_INC_DIR)
CFLAGS = -g
LDFLAGS = -L$(RAP_LIB_DIR) -ltdrp

SRCS = \
	_tdrp.c \
	write_result.c \
	parse_args.c \
	area_compute.c

OBJS = $(SRCS:.c=.o)

# link

$(PROGNAME): $(OBJS)
	$(CC) -o $(PROGNAME) $(OBJS) $(LDFLAGS)

# tdrp

_tdrp.c: paramdef.$(PROGNAME)
	tdrp_gen -f paramdef.$(PROGNAME) -prog $(PROGNAME)

clean_tdrp:
	$(RM) _tdrp.h _tdrp.c

clean:
	$(RM) core a.out
	$(RM) *.i *.o  *.ln *~

clean_bin:
	$(RM) $(PROGNAME)

clean_all: clean clean_bin clean_tdrp

# suffix rules

.SUFFIXES: .c .o

.c.o:
	$(CC) -c $(CFLAGS) $(INCLUDES) $<


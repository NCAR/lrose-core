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
# Makefile for n_xtract program
#
# Nancy Rehak, RAP, NCAR
# POBox 3000, Boulder, CO, 80307, USA
#
# January 2007
#
###########################################################################

TARGET_FILE = n_xtract
INSTALL_PATH = $(REFR_BIN_DIR)/$(TARGET_FILE)

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
MACH_OS = LINUX
INCLUDES = -I$(REFR_INC_DIR)
DEBUG_FLAGS = -g
OPT_FLAGS = -O4
LDFLAGS = -L$(REFR_LIB_DIR)
LIBS = -lrefractt

SRCS = \
	n_output.cc \
	n_output_spol.cc \
	n_smooth.cc \
	n_xtract.cc

OBJS = $(SRCS:.cc=.o)

all:
	$(MAKE) CFLAGS="$(DEBUG_FLAGS)" $(TARGET_FILE)

opt:
	$(MAKE) CFLAGS="$(OPT_FLAGS)" $(TARGET_FILE)

static:
	$(MAKE) CFLAGS="$(DEBUG_FLAGS) -static" $(TARGET_FILE)

staticopt:
	$(MAKE) CFLAGS="$(OPT_FLAGS) -static" $(TARGET_FILE)

install: $(INSTALL_PATH)

$(INSTALL_PATH): all
	if $(TEST) -d $(REFR_BIN_DIR); then :; else \
		$(MKDIR) $(REFR_BIN_DIR); fi ; \
	if $(TEST) -f $(INSTALL_PATH) ;\
	then \
	 	$(MV) $(REFR_BIN_DIR)/$(TARGET_FILE) $(REFR_BIN_DIR)/$(TARGET_FILE).bak ;\
		$(CP) $(TARGET_FILE) $(INSTALL_PATH) ;\
		$(RM) $(REFR_BIN_DIR)/$(TARGET_FILE).bak ;\
	else \
		$(CP) $(TARGET_FILE) $(INSTALL_PATH); \
	fi

$(TARGET_FILE): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET_FILE)  $(OBJS) $(LDFLAGS) $(LIBS)

Params.hh: Params.cc

Params.cc: paramdef.$(TARGET_FILE)
	tdrp_gen -f paramdef.$(TARGET_FILE) -c++ -prog $(TARGET_FILE)

clean:
	$(RM) core a.out *.o *~ junk*

clean_bin:
	$(RM) $(TARGET_FILE)

clean_all: clean_bin clean

clean_tdrp:
	$(RM) Params.cc Params.hh

.SUFFIXES: .cc

.cc.o:
	$(CC) $(CFLAGS) -c $(INCLUDES) -D${MACH_OS} -DSYSV $<

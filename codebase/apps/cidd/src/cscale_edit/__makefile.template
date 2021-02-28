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
# Makefile for CIDD program Utility CSCALE_EDIT
#
# Frank Hage RAP, NCAR, Boulder, CO, 80307, USA
#
###########################################################################

include $(LROSE_CORE_DIR)/build/make_include/lrose_make_macros

MODULE_TYPE=program

TARGET_FILE = cscale_edit

LOC_INCLUDES = $(SYS_XVIEW_INCLUDES) $(SYS_X_INCLUDES) 
LOC_CFLAGS = 
LOC_CPPFLAGS =
LOC_LDFLAGS =  $(SYS_XVIEW_LDFLAGS) $(SYS_X_LDFLAGS)
LOC_LIBS = \
	-ltoolsa -ldevguide -lxview -lolgx \
	-lX11 -lbz2 -lz -lpthread \
	-lm

HDRS =  cscale_edit.h \
	cscale_edit_ui.h \
	linear_ui.h \
	read_ui.h \
	write_ui.h

SRCS =  cscale_edit.c \
	cscale_edit_stubs.c \
	cscale_edit_ui.c \
        linear_stubs.c \
        linear_ui.c \
        read_stubs.c \
	read_ui.c \
	write_stubs.c \
	write_ui.c

#
# objects and lint files
#

OBJS = $(SRCS:.c=.o)
LLNS = $(SRCS:.c=.ln)

#
# standard targets
#

include $(LROSE_CORE_DIR)/build/make_include/lrose_make_targets

#
# local targets
#

depend: depend_generic

# DO NOT DELETE THIS LINE -- make depend depends on it.


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
# Makefile for mdv_thresh_cnt programs
# from Makefile for MDV programs
#
# Frank Hage RAP, NCAR, Boulder, CO, 80307, USA
#
# Modified for mdv_thresh_cnt  M. Petach 17-DEC-1998
#
###########################################################################

include $(LROSE_CORE_DIR)/build/make_include/lrose_make_macros

MODULE_TYPE = program

TARGET_FILE = mdv_thresh_cnt

LOC_INCLUDES =
LOC_CFLAGS = -g
LOC_LDFLAGS =
LOC_LIBS = \
	-ldidss -lmdv -lrapformats -ltoolsa -lpthread \
	-ldataport -ltdrp -lbz2 -lz \
	-lm

HDRS = \
	_tdrp.h \
	mdv_thresh_cnt.h

SRCS = \
	_tdrp.c \
	mdv_thresh_cnt.c

OBJS = \
	_tdrp.o \
	mdv_thresh_cnt.o

#
# standard targets
#

include $(LROSE_CORE_DIR)/build/make_include/lrose_make_c_targets

#
# local targets
#

mdv_thresh_cnt.c: mdv_thresh_cnt.h

depend: depend_generic

#
# tdrp support
#

_tdrp.c: paramdef.$(TARGET_FILE)
	tdrp_gen -f paramdef.$(TARGET_FILE) -prog $(TARGET_FILE)

clean_tdrp:
	$(RM) _tdrp.h _tdrp.c

# DO NOT DELETE THIS LINE -- make depend depends on it.


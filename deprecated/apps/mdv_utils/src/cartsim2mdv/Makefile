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
# Makefile for cartsim2mdv 
#
# Dave Albo June 1997
#
###########################################################################

include $(LROSE_CORE_DIR)/build/make_include/lrose_make_macros

MODULE_TYPE=progcpp

TARGET_FILE = cartsim2mdv

LOC_INCLUDES = -I..
LOC_CFLAGS = 
LOC_CPPC_CFLAGS =
LOC_LDFLAGS =

LOC_LIBS = \
	-lsim -lrsbutil -lsim -lmdv \
	-lrapformats -ltoolsa -lpthread -ldataport -lctetwws \
	-ltdrp -lbz2 -lz -lm

HDRS = \
	cartsim2mdv.h 

C_SRCS = \
	$(TDRP_C)

CPPC_SRCS = \
	parse_args.cc \
	cartsim.cc \
	cartsim_le.cc \
	process.cc \
	cartsim2mdv.cc 
#
# tdrp support
#
include $(LROSE_CORE_DIR)/build/make_include/lrose_make_tdrp_macros

#
# standard targets
#

include $(LROSE_CORE_DIR)/build/make_include/lrose_make_targets

#
# local targets
#

depend: $(TDRP_C) depend_generic

$(TDRP_C): $(TDRP_DEFS)
	tdrp_gen $(TDRP_MODULE) -f $(TDRP_DEFS)

clean_tdrp::
	$(RM) $(TDRP_HDRS) $(TDRP_C)

depend: depend_generic


# DO NOT DELETE THIS LINE -- make depend depends on it.


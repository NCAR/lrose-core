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
# Makefile for trec_gauge_spdb2symprod program
#
# Mike Dixon, RAP, NCAR, Boulder, CO, 80307, USA
#
# Sept 1997
#
###########################################################################

include $(RAP_MAKE_INC_DIR)/rap_make_macros

MODULE_TYPE=progcpp

TARGET_FILE = trec_gauge_spdb2symprod

LOC_INCLUDES =
LOC_CFLAGS =
LOC_CPPC_CFLAGS = 
LOC_LDFLAGS = 
LOC_LIBS = \
	-lSpdbServer -lsymprod -ltitan -lmdv \
	-lrapformats -ltoolsa -ldataport -ltdrp \
	-lbz2 -lz -lm

HDRS = 

C_SRCS = \
	$(TDRP_C) 

CPPC_SRCS = \
	add_to_buffer.cc \
	convert_to_symprod.cc \
	convert_utils.cc \
	handle_sigpipe.cc \
	parse_args.cc \
	tidy_and_exit.cc \
	trec_gauge_spdb2symprod.cc

#
# tdrp support
#

include $(RAP_MAKE_INC_DIR)/rap_make_tdrp_macros

#
# standard targets
#

include $(RAP_MAKE_INC_DIR)/rap_make_targets

#
# local targets
#

depend: $(TDRP_C) depend_generic

include $(RAP_MAKE_INC_DIR)/rap_make_tdrp_targets

# DO NOT DELETE THIS LINE -- make depend depends on it.


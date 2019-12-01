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
# Makefile for wmi_ac_recover program
#
# Mike Dixon, RAP, NCAR, Boulder, CO, 80307, USA
#
# June 1996
#
###########################################################################

include $(RAP_MAKE_INC_DIR)/rap_make_macros

TARGET_FILE = wmi_ac_recover

LOC_INCLUDES =
LOC_CFLAGS =
LOC_LDFLAGS =
LOC_LIBS = \
	-ltitan -lrapformats -lsymprod -ltoolsa -lpthread \
	-ltdrp -ldataport -ltdrp -lbz2 \
	-lz -lm

HDRS = wmi_ac_recover.h

SRCS = \
	$(TDRP_C) \
	calc_latlon.c	   \
	calc_pres.c	   \
	calc_palt.c	   \
	exit_str.c	   \
	fill_acqtbl.c	   \
	fill_cvars.c	   \
	fill_fcoeff.c      \
	find_acqvar.c      \
	find_cvar.c	   \
	parse_args.c       \
	parse_raw_data.c   \
	read_tables.c      \
	store_pos.c        \
	swap_ddir.c        \
	wmi_ac_recover.c  

#
# tdrp support
#

include $(RAP_MAKE_INC_DIR)/rap_make_tdrp_macros

#
# standard targets
#

include $(RAP_MAKE_INC_DIR)/rap_make_c_targets

#
# local targets
#

depend: $(TDRP_C) depend_generic

include $(RAP_MAKE_INC_DIR)/rap_make_tdrp_targets

# DO NOT DELETE THIS LINE -- make depend depends on it.


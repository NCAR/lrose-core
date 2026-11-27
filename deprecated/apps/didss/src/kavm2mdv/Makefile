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
# Makefile for kavm2mdv program
#
# Nancy Rehak, RAP, NCAR, Boulder, CO, 80307, USA
#
# January 1997
#
###########################################################################

include $(LROSE_CORE_DIR)/build/make_include/lrose_make_macros

TARGET_FILE = kavm2mdv

LOC_INCLUDES =
LOC_CFLAGS =
LOC_CPPC_CFLAGS =
LOC_LDFLAGS =
LOC_LIBS = \
	-ltitan -lMdv -lrapformats -ltoolsa -lpthread \
	-lrapmath -ldataport -ltdrp -lbz2 \
	-lz -lm

MODULE_TYPE=progcpp

HDRS = kavm2mdv.h

C_SRCS = \
	$(TDRP_C) 

CPPC_SRCS = \
	convert_params.cc \
	get_next_file.cc \
	init_vol_index.cc \
	kavm2mdv.cc \
	load_data_grid.cc \
	parse_args.cc \
	process_file.cc \
	set_vol_time.cc \
	tidy_and_exit.cc \
	write_output.cc

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

include $(LROSE_CORE_DIR)/build/make_include/lrose_make_tdrp_targets

# DO NOT DELETE THIS LINE -- make depend depends on it.


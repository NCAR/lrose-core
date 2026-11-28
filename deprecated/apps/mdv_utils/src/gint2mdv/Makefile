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
# Makefile for gint2mdv 
#
# Rachel Ames, 1/96.
#
###########################################################################

include $(LROSE_CORE_DIR)/build/make_include/lrose_make_macros

MODULE_TYPE=progcpp

TARGET_FILE = gint2mdv

LOC_INCLUDES = -I..
LOC_CFLAGS = 
LOC_CPPC_CFLAGS =
LOC_LDFLAGS =
LOC_LIBS = \
	-lcidd -lmdv -lrapformats -ltoolsa -lpthread \
	-ldataport -ltdrp -lbz2 -lz \
	-lm

HDRS = \
	gint2mdv.h 

C_SRCS = \
	$(TDRP_C)

CPPC_SRCS = \
	create_mdv_file_name.cc \
	create_gint_file_name.cc \
	parse_args.cc \
	process_file.cc \
	get_latest_data_time.cc \
	update_cindex.cc \
	fill_mdv_field_header.cc \
	fill_mdv_chunk_header.cc \
	fill_mdv_vlevel_header.cc \
	fill_mdv_master_header.cc \
	gint2mdv.cc 


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


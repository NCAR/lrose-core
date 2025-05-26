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
# Makefile for ptrec program
#
# Laura Stoltz, RAP, NCAR, Boulder, CO, 80307, USA
#
# February 1996
#
###########################################################################

include $(LROSE_CORE_DIR)/build/make_include/lrose_make_macros

#
# Let's use the c++ compiler so we get the new language extensions, e.g. bool
#

TARGET_FILE = ptrec

LOC_INCLUDES = $(NETCDF4_INCS)
LOC_CFLAGS = 
#LOC_CFLAGS = -O3
LOC_LDFLAGS = 
LOC_LIBS = \
	-ldsserver -ldidss -lcidd -lMdv \
	-lrapformats -ltoolsa -lpthread -ldataport \
	-ltdrp -lbz2 -lz -lm

HDRS = \
	trec.h

C_SRCS = \
	$(TDRP_C) 

CPPC_SRCS = \
	archive_loop.cc \
	azindx.cc \
	create_file_names.cc \
	deci.cc \
	fill3d.cc \
	find_field.cc \
	get_latest_data_time.cc \
	init_ls.cc \
	input_select.cc \
	lstsq.cc \
	parse_args.cc \
	process_files.cc \
	read_params.cc \
	real_loop.cc \
	tidy_and_exit.cc \
	topdir_select.cc \
	tkcomp.cc \
	trec.cc \
	update_cindex.cc \
	vcterp.cc \
	wair.cc \
	write_mdv_file.cc

#
# tdrp support
#

include $(LROSE_CORE_DIR)/build/make_include/lrose_make_tdrp_macros

#
# standard targets
#

include $(LROSE_CORE_DIR)/build/make_include/lrose_make_opt_c++_targets

#
# local targets
#

depend: $(TDRP_C) depend_generic

include $(LROSE_CORE_DIR)/build/make_include/lrose_make_tdrp_targets

# DO NOT DELETE THIS LINE -- make depend depends on it.


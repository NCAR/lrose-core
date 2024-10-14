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
# Makefile for verify_tracks program
#
# Mike Dixon, RAP, NCAR, Boulder, CO, 80307, USA
#
# Nov 1994
#
###########################################################################

include $(LROSE_CORE_DIR)/build/make_include/lrose_make_macros

TARGET_FILE = verify_tracks

LOC_INCLUDES = $(NETCDF4_INCS)

LOC_LIBS = \
	-ltitan -lMdv -lrapformats -ldsserver \
	-ldidss -leuclid -lrapmath -ltoolsa \
	-ldataport -ltdrp -lRadx -lNcxx \
	-lphysics -lpthread $(NETCDF4_LIBS) -lbz2 \
	-lz -lm

LOC_LDFLAGS = $(NETCDF4_LDFLAGS)

LOC_CFLAGS =

MODULE_TYPE=program

HDRS = verify_tracks.h

CPPC_SRCS = \
	compute_contingency_data.cc \
	compute_errors.cc \
	compute_lookup.cc \
	compute_stats.cc \
	debug_print.cc \
	find_last_descendant.cc \
	increment_count.cc \
	init_indices.cc \
	load_file_stats.cc \
	load_grid.cc \
	load_props.cc \
	load_scan_times.cc \
	open_files.cc \
	parse_args.cc \
	perform_verification.cc \
	print_contingency_table.cc \
	print_stats.cc \
	read_params.cc \
	save_verification_parameters.cc \
	set_counts_to_longs.cc \
	tidy_and_exit.cc \
	triangle_points.cc \
	verify_tracks.cc

#
# standard targets
#

include $(LROSE_CORE_DIR)/build/make_include/lrose_make_c_targets
include $(LROSE_CORE_DIR)/build/make_include/lrose_make_tdrp_c_targets

#
# local targets
#

depend: depend_generic
# DO NOT DELETE THIS LINE -- make depend depends on it.



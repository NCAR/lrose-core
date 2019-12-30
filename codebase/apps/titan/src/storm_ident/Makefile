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
# Makefile for storm_ident program
#
# Mike Dixon, RAP, NCAR, Boulder, CO, 80307, USA
#
# Nov 1994
#
###########################################################################

include $(RAP_MAKE_INC_DIR)/rap_make_macros

TARGET_FILE = storm_ident

LOC_INCLUDES =
LOC_CFLAGS = 
LOC_LDFLAGS =
LOC_LIBS = \
	-ltitan -lrapmath -leuclid -lmdv \
	-lrapformats -lphysics -ltoolsa -lpthread -ldataport \
	-ltdrp -lbz2 -lz -lm

HDRS = storm_ident.h

SRCS = \
	$(TDRP_C) \
	area_comps.c \
	alloc_area_coords.c \
	check_second_trip.c \
	composite.c \
	dbz_gradient_compute.c \
	ellipse_compute.c \
	erode_proj_area.c \
	get_time_list.c \
	identify.c \
	load_header.c \
	load_props.c \
	lock_file.c \
	mask_low_tops.c \
	parse_args.c \
	prepare_restart_storm_file.c \
	prepare_storm_file.c \
	process_clumps.c \
	process_this_clump.c \
	props_compute.c \
	read_dobson.c \
	restart.c \
	set_derived_params.c \
	shmem.c \
	storm_ident.c \
	tidy_and_exit.c \
	tilt_compute.c \
	time_limits.c \
	vol_and_area_comps.c \
	write_file_index.c \
	write_morphology_file.c \
	write_verification_file.c

#
# tdrp macros
#

include $(RAP_MAKE_INC_DIR)/rap_make_tdrp_macros

#
# standard C targets
#

include $(RAP_MAKE_INC_DIR)/rap_make_c_targets

#
# tdrp targets
#

include $(RAP_MAKE_INC_DIR)/rap_make_tdrp_targets

#
# local targets
#

depend: depend_generic

# DO NOT DELETE THIS LINE -- make depend depends on it.


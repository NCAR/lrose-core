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
# Makefile for verify_grid program
#
# Mike Dixon, RAP, NCAR, Boulder, CO, 80307, USA
#
# Nov 1994
#
###########################################################################

include $(LROSE_CORE_DIR)/build/make_include/lrose_make_macros

TARGET_FILE = verify_grid

LOC_INCLUDES = $(NETCDF4_INCS)

LOC_LIBS = \
	-ltitan -lMdv -lrapformats \
	-ldsserver -ldidss -leuclid -lrapmath \
	-ltoolsa -ldataport -ltdrp -lRadx \
	-lNcxx -lphysics -lpthread $(NETCDF4_LIBS) \
	-lbz2 -lz -lm

LOC_LDFLAGS = $(NETCDF4_LDFLAGS)

LOC_CFLAGS =

MODULE_TYPE=program

HDRS = verify_grid.h

SRCS = \
	$(TDRP_C) \
	init_indices.c \
	get_truth_path.c \
	init_intermediate_grid.c \
	parse_args.c \
	parse_threshold.c \
	print_cart_params.c \
	print_contingency_table.c \
	print_header.c \
	print_stats.c \
	tidy_and_exit.c \
	update_cont.c \
	update_regression.c \
	update_stats.c \
	verify_grid.c

#
# tdrp support
#

include $(LROSE_CORE_DIR)/build/make_include/lrose_make_tdrp_macros

#
# standard targets
#

include $(LROSE_CORE_DIR)/build/make_include/lrose_make_c_targets

#
# local targets
#

depend: $(TDRP_C) depend_generic

include $(LROSE_CORE_DIR)/build/make_include/lrose_make_tdrp_targets

# DO NOT DELETE THIS LINE -- make depend depends on it.



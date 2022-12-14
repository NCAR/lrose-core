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
# Makefile for TimeHist program
#
# Mike Dixon, RAP, NCAR, Boulder, CO, 80307, USA
#
# Nov 1994
# Converted to C++, Feb 2001
#
###########################################################################

include $(LROSE_CORE_DIR)/build/make_include/lrose_make_macros

MODULE_TYPE=program

TARGET_FILE = TimeHist

LOC_INCLUDES = $(SYS_X_INCLUDES)
LOC_CFLAGS = 
LOC_LDFLAGS = $(SYS_X_LDFLAGS)
LOC_LIBS = \
	-ltitan -lMdv -lrapformats -ldsserver \
	-ldidss -leuclid -lrapmath -lrapplot \
	-ltoolsa -ldataport -ltdrp -lRadx \
	-lNcxx -lphysics -lpthread -lX11 \
	-lbz2 -lz $(NETCDF4_LIBS)

HDRS = TimeHist.hh

CPPC_SRCS = \
	compute_ticks.cc \
	compute_track_num.cc \
	copy.cc \
	create_frames.cc \
	data_check.cc \
	draw_rdist.cc \
	draw_scale.cc \
	draw_thist.cc \
	draw_timeht.cc \
	draw_tscale.cc \
	draw_union.cc \
	event_loop.cc \
	free_resources.cc \
	get_ps_color_scales.cc \
	get_titan_data.cc \
	get_x_color_scales.cc \
	help.cc \
	load_plot_data.cc \
	parse_args.cc \
	print_copy.cc \
	read_params.cc \
	set_sens.cc \
	set_xfonts.cc \
	set_xgcs.cc \
	setup_help_windows.cc \
	setup_rdist_windows.cc \
	setup_thist_windows.cc \
	setup_timeht_windows.cc \
	setup_track_shmem.cc \
	setup_tscale_windows.cc \
	setup_union_windows.cc \
	setup_x.cc \
	tidy_and_exit.cc \
	TimeHist.cc \
	xerror_handler.cc

#
# standard targets
#

include $(LROSE_CORE_DIR)/build/make_include/lrose_make_c++_targets

clean_tdrp:

#
# local targets
#

depend: depend_generic

# DO NOT DELETE THIS LINE -- make depend depends on it.



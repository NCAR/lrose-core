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
# Makefile for time_hist program
#
# Mike Dixon, RAP, NCAR, Boulder, CO, 80307, USA
#
# Nov 1994
#
###########################################################################

include $(LROSE_CORE_DIR)/build/make_include/lrose_make_macros

MODULE_TYPE=program

TARGET_FILE = time_hist

LOC_INCLUDES = $(SYS_X_INCLUDES)
LOC_CFLAGS = 
LOC_LDFLAGS = $(SYS_X_LDFLAGS)
LOC_LIBS = \
	-lrapplot -ltitan -lmdv -lrapformats \
	-lphysics -lrapmath -ltoolsa -ldataport \
	-lX11 -lbz2 -lz -lpthread \
	-lm

HDRS = time_hist.h

SRCS = \
	compute_ticks.c \
	compute_track_num.c \
	copy.c \
	create_frames.c \
	draw_rdist.c \
	draw_scale.c \
	draw_thist.c \
	draw_timeht.c \
	draw_tscale.c \
	event_loop.c \
	free_resources.c \
	get_ps_color_scales.c \
	get_track_data.c \
	get_x_color_scales.c \
	help.c \
	load_plot_data.c \
	parse_args.c \
	print_copy.c \
	read_params.c \
	respond_to_timer.c \
	set_sens.c \
	set_timer.c \
	set_xfonts.c \
	set_xgcs.c \
	setup_help_windows.c \
	setup_rdist_windows.c \
	setup_thist_windows.c \
	setup_timeht_windows.c \
	setup_track_shmem.c \
	setup_tscale_windows.c \
	setup_x.c \
	tidy_and_exit.c \
	time_hist.c \
	xerror_handler.c

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


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
# Makefile for rview program
#
# Mike Dixon, RAP, NCAR, Boulder, CO, 80307, USA
#
# Nov 1994
#
###########################################################################

include $(RAP_MAKE_INC_DIR)/rap_make_macros

MODULE_TYPE=program

TARGET_FILE = rview

LOC_INCLUDES = $(SYS_X_INCLUDES)
LOC_CFLAGS = 
LOC_LDFLAGS = $(SYS_X_LDFLAGS)
LOC_LIBS = \
	-lrapplot -ltitan -lcidd -lsymprod \
	-leuclid -lrapmath -lrapformats -ltoolsa \
	-ldataport -lX11 -lbz2 -lz \
	-lpthread -lm

HDRS = rview.h

SRCS = \
	convert_xwd_to_web.c \
	copy_cappi.c \
	copy_cappi_web.c \
	copy_vsection.c \
	copy_vsection_web.c \
	create_frames.c \
	draw_ac_posn.c \
	draw_cappi.c \
	draw_main_scale.c \
	draw_tracks.c \
	draw_verify.c \
	draw_vsection.c \
	event_loop.c \
	free_resources.c \
	get_contour_intervals.c \
	get_ps_color_scales.c \
	get_track_data.c \
	get_x_color_scales.c \
	help.c \
	map.c \
	parse_args.c \
	print_copy.c \
	read_field_control.c \
	read_params.c \
	respond_to_timer.c \
	rview.c \
	set_sens.c \
	set_timer.c \
	set_xfonts.c \
	set_xgcs.c \
	setup_cappi_page.c \
	setup_cappi_windows.c \
	setup_help_windows.c \
	setup_track_shmem.c \
	setup_vsection_page.c \
	setup_vsection_windows.c \
	setup_x.c \
	tidy_and_exit.c \
	vsection_calc.c \
	xerror_handler.c \
	zoom_calc.c

#
# standard targets
#

include $(RAP_MAKE_INC_DIR)/rap_make_c_targets
include $(RAP_MAKE_INC_DIR)/rap_make_tdrp_c_targets

#
# local targets
#

depend: depend_generic

# DO NOT DELETE THIS LINE -- make depend depends on it.


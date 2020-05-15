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
# Makefile for CIDD program
#
# Frank Hage RAP, NCAR, Boulder, CO, 80307, USA
#
###########################################################################

include $(LROSE_CORE_DIR)/build/make_include/lrose_make_macros

MODULE_TYPE=program

TARGET_FILE = cidd

LOC_INCLUDES = $(SYS_XVIEW_INCLUDES) $(SYS_X_INCLUDES)
LOC_CFLAGS = -D$(HOST_OS)
SYS_CFLAGS = -g -D$(HOST_OS)
LOC_LDFLAGS = $(SYS_XVIEW_LDFLAGS) $(SYS_X_LDFLAGS) 
LOC_LIBS = \
	-lrapplot -lcidd -lrapformats -ldataport \
	-lsymprod -ltoolsa -lpthread -ldevguide -lxview \
	-lolgx -lX11 -lbz2 -lz \
	-lm

HDRS =  cidd.h \
	cidd_func.h \
	cidd_macros.h \
	cidd_structs.h \
	extras.h \
	extras_pu_ui.h \
	h_win_ui.h \
	movie.h \
	movie_pu_ui.h \
	over_pu_ui.h \
	zoom_pu_ui.h \
	data_pu_ui.h \
	save_pu_ui.h \
	spr_98.h \
	v_win_ui.h

SRCS =  cidd.c \
	cidd_colors.c \
	cidd_cbar.c \
        cidd_coords.c \
        cidd_data.c \
        cidd_expti.c \
	cidd_font.c \
	cidd_graphics.c \
	cidd_init.c \
	cidd_movie.c \
	cidd_overlay_init.c \
	cidd_prds.c \
	cidd_render_overlays.c \
	cidd_spr.c \
	cidd_timer.c \
	coord_conv.c \
	extras_pu_proc.c \
	extras_pu_ui.c \
	fields_pu_proc.c \
	fields_pu_ui.c \
	h_win_events.c \
	h_win_proc.c \
	h_win_ui.c \
	movie_pu_proc.c \
	over_pu_ui.c \
	over_pu_proc.c \
	movie_pu_ui.c \
	save_pu_ui.c \
	save_pu_proc.c \
	zoom_pu_ui.c \
	zoom_pu_proc.c \
	data_pu_ui.c \
	data_pu_proc.c \
	render_extras.c \
	render_horiz.c \
	render_products.c \
	render_vert.c\
	render_winds.c \
	spr_98.c \
	v_win_proc.c \
	v_win_ui.c \
	xview_file_access.c


#
# standard targets
#

include $(LROSE_CORE_DIR)/build/make_include/lrose_make_targets

#
# local targets
#

depend: depend_generic

# DO NOT DELETE THIS LINE -- make depend depends on it.


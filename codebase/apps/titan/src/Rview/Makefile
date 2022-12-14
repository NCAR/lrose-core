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
# Makefile for Rview program
#
# Mike Dixon, RAP, NCAR, Boulder, CO, 80307, USA
#
# Nov 1994
#
###########################################################################

include $(LROSE_CORE_DIR)/build/make_include/lrose_make_macros

TARGET_FILE = Rview

LOC_INCLUDES = $(SYS_X_INCLUDES) $(NETCDF4_INCS)

LOC_LIBS = \
	-ltitan -lMdv -lSpdb -lrapformats \
	-ldsserver -ldidss -leuclid -lrapmath \
	-lrapplot -ltoolsa -ldataport -ltdrp \
	-lRadx -lNcxx -lphysics -lpthread \
	-lX11 $(NETCDF4_LIBS) -lbz2 -lz

LOC_LDFLAGS = $(SYS_X_LDFLAGS) $(NETCDF4_LDFLAGS)

LOC_CFLAGS = 

HDRS = \
	ProdParams.hh \
	Rview.hh \
	Product.hh \
	ProductMgr.hh \
	RenderContext.hh \
	SymprodRender.hh \
	SymprodRenderObj.hh

CPPC_SRCS = \
	ProdParams.cc \
	Rview.cc \
	contours.cc \
	convert_xwd_to_web.cc \
	copy_cappi.cc \
	copy_cappi_web.cc \
	copy_vsection.cc \
	copy_vsection_web.cc \
	create_frames.cc \
	data_check.cc \
	draw_ac_posn.cc \
	draw_cappi.cc \
	draw_main_scale.cc \
	draw_titan.cc \
	draw_verify.cc \
	draw_vsection.cc \
	event_loop.cc \
	free_resources.cc \
	get_contour_intervals.cc \
	get_ps_color_scales.cc \
	get_titan_data.cc \
	get_x_color_scales.cc \
	help.cc \
	map.cc \
	parse_args.cc \
	print_copy.cc \
	read_field_control.cc \
	read_params.cc \
	set_sens.cc \
	set_xfonts.cc \
	set_xgcs.cc \
	setup_cappi_page.cc \
	setup_cappi_windows.cc \
	setup_help_windows.cc \
	setup_track_shmem.cc \
	setup_vsection_page.cc \
	setup_vsection_windows.cc \
	setup_x.cc \
	tidy_and_exit.cc \
	xerror_handler.cc \
	zoom_calc.cc \
	ProductMgr.cc \
	Product.cc \
	RenderContext.cc \
	SymprodRender.cc \
	SymprodRenderObj.cc

#
# standard targets
#

include $(LROSE_CORE_DIR)/build/make_include/lrose_make_c++_targets

#
# local targets
#

ProdParams.cc: paramdef.products
	tdrp_gen -f paramdef.products -c++ -class ProdParams \
	-module Products -prog $(TARGET_FILE) -add_ncar_copyright

ProdParams.hh: ProdParams.cc

clean_tdrp:
	$(RM) ProdParams.hh ProdParams.cc

depend: depend_generic

# DO NOT DELETE THIS LINE -- make depend depends on it.



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

TARGET_FILE = CIDD_titan

LAST_INCLUDES = $(SYS_XVIEW_INCLUDES) 

SYS_CFLAGS =
LOC_CPPC_CFLAGS =

LOC_INCLUDES = $(NETCDF4_INCS)

LOC_LIBS = -lSpdb -lMdv -lRadx -lNcxx -leuclid \
	-ldsserver -ldidss -lrapplot -lrapformats \
	-lFmq -ltoolsa -ldataport -ltdrp \
	-lshapelib -ldevguide -lxview -lolgx \
	-lX11 -lXext $(NETCDF4_LIBS) -lbz2 \
	-lz -lpthread

LOC_LDFLAGS = $(SYS_XVIEW_LDFLAGS) $(SYS_X_LDFLAGS) $(NETCDF4_LDFLAGS)


#IMLIB_LIBS =  -lSpdb -lMdv -lRadx -lNcxx -leuclid -ldsserver -ldidss -lrapplot \
#	-lrapformats -lFmq -ltoolsa -ldataport -ltdrp \
#	-lshapelib -lpthread -ldevguide -lxview -lolgx -lX11 -lXext \
#	-lImlib2 -lpng -ltiff -ljpeg -lgif -lz


HDRS =  cidd.h \
	cidd_colorscales.h \
	cidd_contours.h \
	cidd_field_data.h \
	cidd_funcs.h \
	cidd_layers.h \
	cidd_macros.h \
	cidd_movies.h \
	cidd_overlays.h \
	cidd_products.h \
	cidd_structs.h \
	cidd_winds.h \
	cidd_windows.h \
	cmd_pu_ui.h \
	data_pu_ui.h \
	gen_time_pu_ui.h \
	past_pu_ui.h \
	page_pu_ui.h \
	prod_pu_ui.h \
	h_win_ui.h \
	movie_pu_ui.h \
	over_pu_ui.h \
	route_pu_ui.h \
	save_pu_ui.h \
	status_pu_ui.h \
	v_win_ui.h \
	zoom_pu_ui.h \
	Csyprod_P.hh \
	Cdraw_P.hh \
	Cgui_P.hh \
	Cterrain_P.hh \
	Croutes_P.hh \
	Product.hh \
	ProductMgr.hh \
	RenderContext.hh \
	SymprodRender.hh \
	SymprodRenderObj.hh  \
	TimeList.hh \
	TimePlot.hh \
	Uparams.hh


CPPC_SRCS = \
	Csyprod_P.cc \
	Cdraw_P.cc \
	Cgui_P.cc \
	Cterrain_P.cc \
	Croutes_P.cc \
	Product.cc \
	ProductMgr.cc \
	RenderContext.cc \
	SymprodRender.cc \
	SymprodRenderObj.cc \
	TimeList.cc \
	TimePlot.cc \
	cidd_coords.cc \
	cidd_font.cc \
	cidd_init.cc \
	cidd_main.cc \
	cidd_params.cc \
	cidd_remote_commands.cc \
	cidd_reset.cc \
	cidd_signals.cc \
	cmd_pu_proc.cc \
	colorscales.cc \
	contours.cc \
	data_gather.cc \
	data_init.cc \
	data_io.cc \
	data_pu_proc.cc \
	draw_export_init.cc \
	draw_pu_proc.cc \
	past_pu_proc.cc \
	page_pu_proc.cc \
	prod_pu_proc.cc \
	fcast_pu_proc.cc \
	fields_pu_proc.cc \
	gen_time_pu_proc.cc \
	graphic_canvas_events.cc \
	graphic_canvas_events_bkwd.cc \
	graphic_check.cc \
	graphic_compute.cc \
	graphic_cross_section.cc \
	graphic_dump_image.cc \
	graphic_manage.cc \
	graphic_margin_events.cc \
	graphic_panel.cc \
	graphic_reset.cc \
	graphic_resize.cc \
	graphic_xsect_canvas_events.cc \
	graphic_xsect_resize.cc \
	graphic_zoom.cc \
	gui_destroy.cc \
	gui_init.cc \
	gui_label_frame.cc \
	gui_modify.cc \
	h_win_proc.cc  \
	mdvx_data_request.cc \
	movie_control.cc \
	movie_frame_retrieve.cc \
	movie_frame_save.cc \
	movie_pu_proc.cc \
	overlay_init.cc \
	over_pu_proc.cc \
	process_args.cc \
	render_azmiuths.cc \
	render_bottom_margin.cc \
	render_cart_grid.cc \
	render_cbar.cc \
	render_control.cc \
	render_distorted_grid.cc \
	render_filled_contours.cc \
	render_filled_image.cc \
	render_grids.cc \
	render_ht_sel.cc \
	render_left_margin.cc \
	render_legends.cc \
	render_line_contours.cc \
	render_margins.cc \
	render_movie_frame.cc \
	render_overlays.cc  \
	render_polar_grid.cc \
	render_products.cc \
	render_range_rings.cc \
	render_right_margin.cc \
	render_route_winds.cc \
	render_terrain.cc \
	render_top_layers.cc \
	render_top_margin.cc \
	render_winds.cc \
	render_xsect_control.cc \
	render_xsect_grids.cc \
	render_xsect_line_contours.cc \
	render_xsect_margins.cc \
	render_xsect_products.cc \
	render_xsect_top_layers.cc \
	render_xsect_winds.cc \
	route_pu_proc.cc \
	route_winds_init.cc \
	save_pu_proc.cc \
	shmem_init.cc \
	status_pu_proc.cc \
	symprod_init.cc \
	timer_control.cc \
	v_win_proc.cc \
	winds_init.cc \
	xview_file_access.cc \
	zoom_pu_proc.cc \
	cmd_pu_ui.cc \
	data_pu_ui.cc \
	draw_pu_ui.cc \
	fcast_pu_ui.cc \
	fields_pu_ui.cc \
	gen_time_pu_ui.cc \
	h_win_ui.cc \
	movie_pu_ui.cc \
	over_pu_ui.cc \
	past_pu_ui.cc \
	page_pu_ui.cc \
	prod_pu_ui.cc \
	route_pu_ui.cc \
	save_pu_ui.cc \
	status_pu_ui.cc \
	v_win_ui.cc \
	zoom_pu_ui.cc \
	Uparams.cc

C_SRCS =  

#
# TDRP support
#
  
include $(LROSE_CORE_DIR)/build/make_include/lrose_make_tdrp_macros

#
# standard targets
#

include $(LROSE_CORE_DIR)/build/make_include/lrose_make_c++_targets

#
# local targets
#

Csyprod_P.cc: paramdef.Cidd_syprods
	tdrp_gen -f paramdef.Cidd_syprods -c++ -class Csyprod_P  -prog CIDD

Cgui_P.cc: paramdef.Cidd_gui
	tdrp_gen -f paramdef.Cidd_gui -c++ -class Cgui_P  -prog CIDD

Cdraw_P.cc: paramdef.Cidd_draw
	tdrp_gen -f paramdef.Cidd_draw -c++ -class Cdraw_P  -prog CIDD

Cterrain_P.cc: paramdef.Cidd_terrain
	tdrp_gen -f paramdef.Cidd_terrain -c++ -class Cterrain_P  -prog CIDD

Croutes_P.cc: paramdef.Cidd_routes
	tdrp_gen -f paramdef.Cidd_routes -c++ -class Croutes_P  -prog CIDD

depend: cidd_macros.h $(TDRP_C) depend_generic

clean_tdrp:
	$(RM) Csyprod_P.cc Csyprod_P.hh \
	Cdraw_P.cc Cdraw_P.hh \
	Cterrain_P.cc  Cterrain_P.hh \
	Croutes_P.cc Croutes_P.hh

all_imlib:
	$(MAKE) LOC_LIBS="$(IMLIB_LIBS)" LOC_CPPC_CFLAGS=-D__HAVE_IMLIB all

opt_imlib:
	$(MAKE) LOC_LIBS="$(IMLIB_LIBS)" LOC_CPPC_CFLAGS=-D__HAVE_IMLIB  opt


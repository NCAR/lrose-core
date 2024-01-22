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
# Frank Hage EOL, NCAR, Boulder, CO, 80307, USA
#
###########################################################################

include $(LROSE_CORE_DIR)/build/make_include/lrose_make_macros

TARGET_FILE = Qucid

LOC_INCLUDES = $(QT_INCLUDES) $(NETCDF4_INCS)

# Note: QJSENGINE is defined in lrose_make.OSX_LINUX

LOC_LIBS = \
	-ldsdata -lradar -lMdv -lSpdb \
	-lFmq -lrapformats -ldsserver -ldidss \
	-leuclid -lrapmath -lrapplot -lqtplot -ltoolsa \
	-ldataport -ltdrp -lRadx -lNcxx \
	-lphysics -lshapelib \
	$(QT_LIBS) $(QJSENGINE) $(NETCDF4_LIBS) -lX11 -lXext \
	-lfftw3 -lpng -lbz2 -lz -lpthread

LOC_LDFLAGS = -L$(LROSE_INSTALL_DIR)/lib $(QT_LDFLAGS) $(NETCDF4_LDFLAGS)

LOC_CPPC_CFLAGS = $(QT_CPPC_CFLAGS)

HDRS = \
	$(PARAMS_HH) \
	Args.hh \
	CartManager.hh \
	CartWidget.hh \
	Cdraw_P.hh \
	Cgui_P.hh \
	Cimages_P.hh \
	ColorTableManager.hh \
	Croutes_P.hh \
	Csyprod_P.hh \
	Cterrain_P.hh \
	DisplayField.hh \
	DisplayManager.hh \
	HorizWidget.hh \
	LegacyParams.hh \
	Product.hh \
	ProductMgr.hh \
	Qucid.hh \
	Reader.hh \
	RenderContext.hh \
	ScaledLabel.hh \
	SiiPalette.hh \
	SoloDefaultColorWrapper.hh \
	SymprodRender.hh \
	SymprodRenderObj.hh \
	TimeList.hh \
	TimePlot.hh \
	Uparams.hh \
	VertWidget.hh \
	VertWindow.hh \
	WorldPlot.hh \
	cidd.h \
	cidd_colorscales.h \
	cidd_contours.h \
	cidd_data_io.h \
	cidd_dpd.h \
	cidd_field_data.h \
	cidd_funcs.h \
	cidd_layers.h \
	cidd_legend.h \
	cidd_macros.h \
	cidd_movies.h \
	cidd_overlays.h \
	cidd_params.h \
	cidd_products.h \
	cidd_structs.h \
	cidd_windows.h \
	cidd_winds.h

NORM_SRCS = \
	$(PARAMS_CC) \
	Args.cc \
	Cdraw_P.cc \
	Cgui_P.cc \
	Cimages_P.cc \
	ColorTableManager.cc \
	Croutes_P.cc \
	Csyprod_P.cc \
	Cterrain_P.cc \
	DisplayField.cc \
	LegacyParams.cc \
	Main.cc \
	Product.cc \
	ProductMgr.cc \
	Qucid.cc \
	Reader.cc \
	RenderContext.cc \
	ScaledLabel.cc \
	SiiPalette.cc \
	SoloDefaultColorWrapper.cc \
	SymprodRender.cc \
	SymprodRenderObj.cc \
	TimeList.cc \
	TimePlot.cc \
	Uparams.cc \
	WorldPlot.cc \
	cidd_coords.cc \
	cidd_font.cc \
	cidd_init.cc \
	cidd_params.cc \
	cidd_remote_commands.cc \
	cidd_reset.cc \
	cidd_signals.cc \
	colorscales.cc \
	contours.cc \
	data_gather.cc \
	data_init.cc \
	data_io.cc \
	data_pu_proc.cc \
	draw_export_init.cc \
	draw_pu_proc.cc \
	fields_pu_proc.cc \
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
	h_win_proc.cc \
	mdvx_data_request.cc \
	movie_control.cc \
	movie_frame_retrieve.cc \
	movie_frame_save.cc \
	movie_pu_proc.cc \
	overlay_init.cc \
	page_pu_proc.cc \
	process_args.cc \
	render_azimuths.cc \
	render_bottom_margin.cc \
	render_cart_grid.cc \
	render_cbar.cc \
	render_click_marks.cc \
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
	render_overlays.cc \
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
	route_winds_init.cc \
	shmem_init.cc \
	status_pu_proc.cc \
	symprod_init.cc \
	timer_control.cc \
	v_win_proc.cc \
	winds_init.cc \
	xview_file_access.cc \
	zoom_pu_proc.cc

MOC_SRCS = \
	CartManager.cc \
	CartWidget.cc \
	DisplayManager.cc \
	HorizWidget.cc \
	VertWidget.cc \
	VertWindow.cc

MOC_OUTPUT = \
	moc_CartManager.cpp \
	moc_CartWidget.cpp \
	moc_DisplayManager.cpp \
	moc_HorizWidget.cpp \
	moc_VertWidget.cpp \
	moc_VertWindow.cpp

CC_SRCS = $(NORM_SRCS) $(MOC_SRCS)
CPP_SRCS = $(MOC_OUTPUT)

#
# tdrp macros
#

include $(LROSE_CORE_DIR)/build/make_include/lrose_make_tdrp_macros

#
# QT C++ targets
#

include $(LROSE_CORE_DIR)/build/make_include/lrose_make_qt_targets

#
# tdrp targets
#

include $(LROSE_CORE_DIR)/build/make_include/lrose_make_tdrp_c++_targets

#
# local targets
#

#Csyprod_P.cc: paramdef.Cidd_syprods
#	tdrp_gen -f paramdef.Cidd_syprods -c++ -class Csyprod_P -prog CIDD -add_ncar_copyright

#Cgui_P.cc: paramdef.Cidd_gui
#	tdrp_gen -f paramdef.Cidd_gui -c++ -class Cgui_P -prog CIDD -add_ncar_copyright

#Cimages_P.cc: paramdef.Cidd_images
#	tdrp_gen -f paramdef.Cidd_images -c++ -class Cimages_P -prog CIDD -add_ncar_copyright

#Cdraw_P.cc: paramdef.Cidd_draw
#	tdrp_gen -f paramdef.Cidd_draw -c++ -class Cdraw_P -prog CIDD -add_ncar_copyright

#Cterrain_P.cc: paramdef.Cidd_terrain
#	tdrp_gen -f paramdef.Cidd_terrain -c++ -class Cterrain_P -prog CIDD -add_ncar_copyright

#Croutes_P.cc: paramdef.Cidd_routes
#	tdrp_gen -f paramdef.Cidd_routes -c++ -class Croutes_P -prog CIDD -add_ncar_copyright

depend: cidd_macros.h $(TDRP_C) depend_generic

#clean_tdrp:
#	$(RM) Csyprod_P.cc Csyprod_P.hh \
#	Cdraw_P.cc Cdraw_P.hh \
#	Cgui_P.cc Cgui_P.hh \
#	Cimages_P.cc Cimages_P.hh \
#	Cterrain_P.cc  Cterrain_P.hh \
#	Croutes_P.cc Croutes_P.hh

# DO NOT DELETE THIS LINE -- make depend depends on it.


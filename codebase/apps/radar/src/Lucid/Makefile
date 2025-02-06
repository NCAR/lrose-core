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

TARGET_FILE = Lucid

LOC_INCLUDES = $(QT_INCLUDES) $(NETCDF4_INCS)

# Note: QJSENGINE is defined in lrose_make.OSX_LINUX

LOC_LIBS = \
	-ldsdata -lradar -lMdv -lSpdb \
	-lFmq -lrapformats -ldsserver -ldidss \
	-leuclid -lrapmath -lrapplot -lqtplot -ltoolsa \
	-ldataport -ltdrp -lRadx -lNcxx \
	-lphysics -lshapelib \
	$(QT_LIBS) $(QJSENGINE) $(NETCDF4_LIBS) -lX11 -lXext \
	-lfftw3 -lpng -lpthread -lz -lbz2

LOC_LDFLAGS = -L$(LROSE_INSTALL_DIR)/lib $(QT_LDFLAGS) $(NETCDF4_LDFLAGS)

LOC_CPPC_CFLAGS = $(QT_CPPC_CFLAGS)

HDRS = \
	$(PARAMS_HH) \
	Args.hh \
	GuiManager.hh \
	Cdraw_P.hh \
	Cgui_P.hh \
	Cimages_P.hh \
	Croutes_P.hh \
	Csyprod_P.hh \
	Cterrain_P.hh \
	FieldTableItem.hh \
	HorizWidget.hh \
	LegacyParams.hh \
	MapMenuItem.hh \
	MetRecord.hh \
	Product.hh \
	ProductMgr.hh \
	ProdMenuItem.hh \
	LatLonBox.hh \
	Lucid.hh \
	RenderContext.hh \
	ScaledLabel.hh \
	SymprodRender.hh \
	SymprodRenderObj.hh \
	TimeList.hh \
	VertWidget.hh \
	VertWindow.hh \
	VlevelManager.hh \
	WayPts.hh \
	WindMenuItem.hh \
	WorldPlot.hh \
	XyBox.hh \
	ZoomMenuItem.hh \
	cidd.h \
	cidd_colorscales.h \
	cidd_contours.h \
	cidd_data_io.h \
	cidd_dpd.h \
	cidd_funcs.h \
	cidd_layers.h \
	cidd_legend.h \
	cidd_macros.h \
	cidd_movies.h \
	cidd_overlays.h \
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
	Croutes_P.cc \
	Csyprod_P.cc \
	Cterrain_P.cc \
	FieldTableItem.cc \
	LatLonBox.cc \
	LegacyParams.cc \
	Main.cc \
	Product.cc \
	ProductMgr.cc \
	RenderContext.cc \
	ScaledLabel.cc \
	SymprodRender.cc \
	SymprodRenderObj.cc \
	TimeList.cc \
	VlevelManager.cc \
	WayPts.cc \
	WorldPlot.cc \
	XyBox.cc

MOC_SRCS = \
	Lucid.cc \
	GuiManager.cc \
	HorizWidget.cc \
	MapMenuItem.cc \
	MdvReader.cc \
	ProdMenuItem.cc \
	TimeControl.cc \
	VertWidget.cc \
	VertWindow.cc \
	WindMenuItem.cc \
	ZoomMenuItem.cc

MOC_OUTPUT = \
	moc_Lucid.cpp \
	moc_GuiManager.cpp \
	moc_HorizWidget.cpp \
	moc_MapMenuItem.cpp \
	moc_MdvReader.cpp \
	moc_ProdMenuItem.cpp \
	moc_TimeControl.cpp \
	moc_VertWidget.cpp \
	moc_VertWindow.cpp \
	moc_WindMenuItem.cpp \
	moc_ZoomMenuItem.cpp

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


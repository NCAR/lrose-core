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
# Makefile for HawkEye program
#
# Mike Dixon, RAP, NCAR
# POBox 3000, Boulder, CO, 80307, USA
#
# May 2006
#
###########################################################################

include $(RAP_MAKE_INC_DIR)/rap_make_macros

TARGET_FILE = HawkEye

LOC_INCLUDES = $(QT_INCLUDES) $(NETCDF4_INCS)

LOC_LIBS = -lSpdb -lradar -lrapformats -lFmq -lRadx \
	-ldsserver -ldidss -leuclid -lphysics \
	-lrapmath -ltoolsa -ldataport -ltdrp \
	$(QT_LIBS) $(NETCDF4_LIBS) \
	-lfftw3 -lbz2 -lz -lpthread -lm

LOC_LDFLAGS = -L$(LROSE_INSTALL_DIR)/lib $(QT_LDFLAGS) $(NETCDF4_LDFLAGS)

LOC_CPPC_CFLAGS = $(QT_CPPC_CFLAGS) #  -DSMART_COLOR_SCALES

HDRS = \
	$(PARAMS_HH) \
	Args.hh \
	Beam.hh \
	BscanBeam.hh \
	BscanManager.hh \
	BscanWidget.hh \
	ColorMap.hh \
	ColorTableManager.hh \
	DisplayManager.hh \
	FieldRenderer.hh \
	HawkEye.hh \
	PaletteManager.hh \
	Params.hh \
	PpiBeam.hh \
	PolarManager.hh \
	PolarWidget.hh \
	PpiWidget.hh \
	Reader.hh \
	RhiBeam.hh \
	RhiWidget.hh \
	RhiWindow.hh \
	ScaledLabel.hh \
	SiiPalette.hh \
	SoloDefaultColorWrapper.hh \
	SweepManager.hh \
	TimeScaleWidget.hh \
	WorldPlot.hh

NORM_SRCS = \
	$(PARAMS_CC) \
	Args.cc \
	Beam.cc \
	BscanBeam.cc \
	ColorMap.cc \
	ColorTableManager.cc \
	DisplayField.cc \
	HawkEye.cc \
	Main.cc \
	PaletteManager.cc \
	PpiBeam.cc \
	Reader.cc \
	RhiBeam.cc \
	ScaledLabel.cc \
	SiiPalette.cc \
	SoloDefaultColorWrapper.cc \
	SweepManager.cc \
	WorldPlot.cc \
	AllocCheck.cc

MOC_SRCS = \
	BscanManager.cc \
	BscanWidget.cc \
	FieldRenderer.cc \
	DisplayManager.cc \
	PolarManager.cc \
	PolarWidget.cc \
	PpiWidget.cc \
	RhiWidget.cc \
	RhiWindow.cc \
	TimeScaleWidget.cc \

MOC_OUTPUT = \
	moc_BscanManager.cpp \
	moc_BscanWidget.cpp \
	moc_DisplayManager.cpp \
	moc_FieldRenderer.cpp \
	moc_PolarManager.cpp \
	moc_PolarWidget.cpp \
	moc_PpiWidget.cpp \
	moc_RhiWidget.cpp \
	moc_RhiWindow.cpp \
	moc_TimeScaleWidget.cpp

CC_SRCS = $(NORM_SRCS) $(MOC_SRCS)
CPP_SRCS = $(MOC_OUTPUT)

#
# tdrp macros
#

include $(RAP_MAKE_INC_DIR)/rap_make_tdrp_macros

#
# QT C++ targets
#

include $(RAP_MAKE_INC_DIR)/rap_make_qt_targets

#
# tdrp targets
#

include $(RAP_MAKE_INC_DIR)/rap_make_tdrp_c++_targets

#
# local targets
#
# in order to build SiiTest_Main, comment any calls to Qt and QBrush, etc. in ColorMap.cc/.hh
#
SiiTest_Main: ColorMap.o SiiPalette.o SiiTest_Main.o SoloDefaultColorWrapper.o PaletteManager.o ColorTableManager.o
	g++ -o SiiTest_Main SiiPalette.o SiiTest_Main.o SoloDefaultColorWrapper.o PaletteManager.o ColorTableManager.o ColorMap.o -L/Users/brenda/lrose/lib -ltoolsa -ldataport -ltdrp  -L/usr/local/Cellar/qt/5.10.1/lib -F/usr/local/Cellar/qt/5.10.1/lib -framework QtWidgets -F/usr/local/Cellar/qt/5.10.1/lib -framework QtGui -F/usr/local/Cellar/qt/5.10.1/lib -framework QtCore -lz -lm -g

#SiiTest_MainNoQt: ColorMap.o SiiPalette.o SiiTest_Main.o SoloDefaultColorWrapper.o PaletteManager.o ColorTableManager.o
	#g++ -o SiiTest_Main SiiPalette.o SiiTest_Main.o SoloDefaultColorWrapper.o PaletteManager.o ColorTableManager.o ColorMap.o -L/Users/brenda/lrose/lib -ltoolsa -ldataport -ltdrp -lz -lm -g

SiiTest_MainNoQt: 
	g++ -o SiiTest_Main SiiPalette.cc SiiTest_Main.cc SoloDefaultColorWrapper.cc PaletteManager.cc ColorTableManager.cc ColorMap.cc -I/Users/brenda/lrose/include -L/Users/brenda/lrose/lib -ltoolsa -ldataport -ltdrp -lz -lm -g

#	g++ -o SiiTest_Main SiiPalette.cc SiiTest_Main.cc SoloDefaultColorWrapper.cc PaletteManager.cc ColorTableManager.cc ColorMap.cc -I/Users/brenda/lrose/include -L/Users/brenda/lrose/lib -ltoolsa -ldataport -ltdrp -lz -lm -g

# DO NOT DELETE THIS LINE -- make depend depends on it.


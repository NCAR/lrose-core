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

include $(LROSE_CORE_DIR)/build/make_include/lrose_make_macros

TARGET_FILE = HawkEye

LOC_INCLUDES = $(QT_INCLUDES) $(NETCDF4_INCS)

# Note: QJSENGINE is defined in lrose_make.OSX_LINUX

LOC_LIBS = \
          -ldsdata -lradar -lFmq -lSpdb -lMdv  -ldsserver -ldidss \
          -lRadx -lNcxx -lrapformats -leuclid -lrapmath -lqtplot \
          -lphysics -ltoolsa -ldataport -ltdrp \
          $(QT_LIBS) $(QJSENGINE) $(NETCDF4_LIBS) \
          -lfftw3 -lbz2 -lz -lpthread

LOC_LDFLAGS = -L$(LROSE_INSTALL_DIR)/lib $(QT_LDFLAGS) $(NETCDF4_LDFLAGS)

LOC_CPPC_CFLAGS = $(QT_CPPC_CFLAGS)

HDRS = \
	$(PARAMS_HH) \
	Args.hh \
	BoundaryPointEditor.hh \
	Beam.hh \
	BscanBeam.hh \
	BscanManager.hh \
	BscanWidget.hh \
	ColorBar.hh \
	ColorTableManager.hh \
	DisplayManager.hh \
	FieldRenderer.hh \
	FlowLayout.hh \
	HawkEye.hh \
	HawkEyeLogger.hh \
	PaletteManager.hh \
	ParameterColorView.hh \
	FieldColorController.hh \
	DisplayFieldModel.hh \
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
	SoloFunctions.hh \
	SpreadSheetController.hh \
	SpreadSheetItem.hh \
	SpreadSheetModel.hh \
	SpreadSheetUtils.hh \
	SpreadSheetView.hh \
	SweepManager.hh \
	TimeScaleWidget.hh \
	WorldPlot.hh

NORM_SRCS = \
	$(PARAMS_CC) \
	AllocCheck.cc \
	Args.cc \
	BoundaryPointEditor.cc \
	Beam.cc \
	BscanBeam.cc \
	ColorTableManager.cc \
	DisplayField.cc \
	DisplayFieldModel.cc \
	FlowLayout.cc \
	HawkEye.cc \
	HawkEyeLogger.cc \
	Main.cc \
	PaletteManager.cc \
	PpiBeam.cc \
	Reader.cc \
	RhiBeam.cc \
	ScaledLabel.cc \
	SiiPalette.cc \
	SoloDefaultColorWrapper.cc \
	SoloFunctionsModel.cc \
	SpreadSheetItem.cc \
	SpreadSheetModel.cc \
	SpreadSheetUtils.cc \
	SweepManager.cc \
	WorldPlot.cc 

MOC_SRCS = \
	BscanManager.cc \
	BscanWidget.cc \
	ClickableLabel.cc \
	ColorBar.cc \
	ColorMapTemplates.cc \
	DisplayManager.cc \
	FieldColorController.cc \
	FieldRenderer.cc \
	FunctionEditor.cc \
	ParameterColorView.cc \
	PolarManager.cc \
	PolarWidget.cc \
	PpiWidget.cc \
	RhiWidget.cc \
	RhiWindow.cc \
	SoloFunctions.cc \
	SpreadSheetController.cc \
	SpreadSheetDelegate.cc \
	SpreadSheetView.cc \
	TextEdit.cc \
	TimeScaleWidget.cc

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
	moc_TimeScaleWidget.cpp \
	moc_ParameterColorView.cpp \
	moc_FieldColorController.cpp \
	moc_ColorBar.cpp \
	moc_ColorMapTemplates.cpp \
	moc_ClickableLabel.cpp \
	moc_TextEdit.cpp \
	moc_SoloFunctions.cpp \
	moc_SpreadSheetView.cpp \
	moc_SpreadSheetController.cpp \
	moc_SpreadSheetDelegate.cpp \
	moc_FunctionEditor.cpp \
	qrc_resources.cpp \
	qrc_customcompleter.cpp

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
# in order to build SiiTest_Main, comment any calls to Qt and QBrush, etc. in ColorMap.cpp/.hh
#

SiiTest_Main: ColorMap.o SiiPalette.o SiiTest_Main.o SoloDefaultColorWrapper.o PaletteManager.o ColorTableManager.o
	g++ -o SiiTest_Main SiiPalette.o SiiTest_Main.o SoloDefaultColorWrapper.o PaletteManager.o ColorTableManager.o ColorMap.o -L/Users/brenda/lrose/lib -ltoolsa -ldataport -ltdrp  -L/usr/local/Cellar/qt/5.10.1/lib -F/usr/local/Cellar/qt/5.10.1/lib -framework QtWidgets -F/usr/local/Cellar/qt/5.10.1/lib -framework QtGui -F/usr/local/Cellar/qt/5.10.1/lib -framework QtCore -lz -lm -g

#SiiTest_MainNoQt: ColorMap.o SiiPalette.o SiiTest_Main.o SoloDefaultColorWrapper.o PaletteManager.o ColorTableManager.o
#g++ -o SiiTest_Main SiiPalette.o SiiTest_Main.o SoloDefaultColorWrapper.o PaletteManager.o ColorTableManager.o ColorMap.o -L/Users/brenda/lrose/lib -ltoolsa -ldataport -ltdrp -lz -lm -g

SiiTest_MainNoQt: 
	g++ -o SiiTest_Main SiiPalette.cc SiiTest_Main.cc SoloDefaultColorWrapper.cc PaletteManager.cc ColorTableManager.cc ColorMap.cc -I/Users/brenda/lrose/include -L/Users/brenda/lrose/lib -ltoolsa -ldataport -ltdrp -lz -lm -g

#   g++ -o SiiTest_Main SiiPalette.cc SiiTest_Main.cc SoloDefaultColorWrapper.cc PaletteManager.cc ColorTableManager.cc ColorMap.cc -I/Users/brenda/lrose/include -L/Users/brenda/lrose/lib -ltoolsa -ldataport -ltdrp -lz -lm -g


#
# build Mac OSX application
#
hawkeye_app:
# Make HawkEye and the Makefile.qmake
	make 
# Make the bundle (HawkEye.app tree structure)
	make from_qmake
# copy the system libraries to the .app tree structure
	mkdir -p HawkEye.app/Contents/Frameworks
	mkdir -p HawkEye.app/Contents/MacOS/../share
	cp -r ~/git/lrose-displays/color_scales HawkEye.app/Contents/MacOS/../share/.
# make the dmg  
	`pkg-config --variable=host_bins Qt5Core`/macdeployqt HawkEye.app -dmg -always-overwrite -codesign="${CERT_NAME}"
# now build the folder for the dmg installer
	hdiutil create -volname HawkEye_Installer -srcfolder ${LROSE_CORE_DIR}/codebase/apps/radar/src/HawkEye/HawkEye.app  -ov -format UDZO HawkEye_Elle.dmg


# DO NOT DELETE THIS LINE -- make depend depends on it.


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
# Makefile for IpsEye program
#
# Mike Dixon, EOL, NCAR
# POBox 3000, Boulder, CO, 80307, USA
#
# May 2006
#
###########################################################################

include $(LROSE_CORE_DIR)/build/make_include/lrose_make_macros

TARGET_FILE = IpsEye

LOC_INCLUDES = $(QT_INCLUDES) $(NETCDF4_INCS)

# Note: QJSENGINE is defined in lrose_make.OSX_LINUX

LOC_LIBS = \
	-ldsdata -lradar -lMdv -lSpdb \
	-lFmq -lrapformats -ldsserver -ldidss \
	-leuclid -lrapmath -ltoolsa -ldataport \
	-ltdrp -lRadx -lNcxx -lphysics \
	$(QT_LIBS) $(QJSENGINE) $(NETCDF4_LIBS) -lfftw3 \
	-lbz2 -lz -lpthread

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
	ColorMap.hh \
	ColorTableManager.hh \
	DisplayManager.hh \
	FieldRenderer.hh \
	FlowLayout.hh \
	IpsEye.hh \
	IpsEyeLogger.hh \
	PaletteManager.hh \
	ParameterColorView.hh \
	FieldColorController.hh \
	DisplayFieldModel.hh \
	Params.hh \
	PpiBeam.hh \
	PolarManager.hh \
	PolarPlot.hh \
	PolarWidget.hh \
	PpiPlot.hh \
	Reader.hh \
	RhiBeam.hh \
	RhiPlot.hh \
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
	ColorMap.cc \
	ColorTableManager.cc \
	DisplayField.cc \
	DisplayFieldModel.cc \
	FlowLayout.cc \
	IpsEye.cc \
	IpsEyeLogger.cc \
	Main.cc \
	PaletteManager.cc \
	PolarPlot.cc \
	PpiBeam.cc \
	PpiPlot.cc \
	Reader.cc \
	RhiBeam.cc \
	RhiPlot.cc \
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
# Make IpsEye and the Makefile.qmake
	make 
# Make the bundle (IpsEye.app tree structure)
	make from_qmake
# copy the system libraries to the .app tree structure
	mkdir -p IpsEye.app/Contents/Frameworks
	cp /usr/lib/libexpat.1.dylib IpsEye.app/Contents/Frameworks/.
	cp /usr/lib/libbz2.1.0.dylib IpsEye.app/Contents/Frameworks/.
	cp /usr/lib/libz.1.dylib IpsEye.app/Contents/Frameworks/.
	cp /usr/lib/libSystem.B.dylib IpsEye.app/Contents/Frameworks/.
	mkdir -p IpsEye.app/Contents/MacOS/../share
	cp -r ~/git/lrose-displays/color_scales IpsEye.app/Contents/MacOS/../share/.
# make the dmg  
	macdeployqt IpsEye.app -dmg -always-overwrite -codesign="${CERT_NAME}"
# now build the folder for the dmg installer
	mkdir -p IpsEye_bkg_test
	cp ~/Desktop/arrow.pdf IpsEye_bkg_test/.background
	rsync -r IpsEye.app IpsEye_bkg_test/.
	hdiutil create -volname IpsEye_Installer4 -srcfolder /Users/brenda/git/lrose-core/codebase/apps/radar/src/IpsEye/IpsEye_bkg_test -ov -format UDZO IpsEye_test4.dmg


# rm -r IpsEye_staging/IpsEye.app
# mkdir -p IpsEye_staging
# cp arrow.pdf IpsEye_staging/.background
# ln -s /Applications IpsEye_staging/Applications
# rsync -r IpsEye.app IpsEye_staging/.
	hdiutil create -volname IpsEye_Installer -srcfolder /Users/brenda/git/lrose-core/codebase/apps/radar/src/IpsEye/IpsEye.app  -ov -format UDZO IpsEye_Cyclone.dmg


# DO NOT DELETE THIS LINE -- make depend depends on it.



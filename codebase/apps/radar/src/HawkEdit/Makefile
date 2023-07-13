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
# POBox 3000,Boulder, CO, 80307, USA
#
# May 2006
#
###########################################################################

include $(LROSE_CORE_DIR)/build/make_include/lrose_make_macros

TARGET_FILE = HawkEdit

LOC_INCLUDES = $(QT_INCLUDES) $(NETCDF4_INCS)

# Note: QJSENGINE is defined in rap_make.OSX_LINUX

LOC_LIBS = -lradar -lMdv -lSpdb -lrapformats -lFmq -lRadx -lSolo -lNcxx \
	-ldsserver -ldidss -leuclid -lphysics \
	-lrapmath -ltoolsa -ldataport -ltdrp \
	$(QT_LIBS) $(QJSENGINE) $(NETCDF4_LIBS) \
	-lfftw3 -lbz2 -lz -lpthread -lm 

LOC_LDFLAGS = -L$(LROSE_INSTALL_DIR)/lib $(QT_LDFLAGS) $(NETCDF4_LDFLAGS)

LOC_CPPC_CFLAGS = -g  $(QT_CPPC_CFLAGS)

# compiler_rcc_make_all: qrc_resources.cpp
compiler_rcc_make_all:
	/usr/local/Cellar/qt/5.15.0/bin/rcc -name resources resources.qrc -o qrc_resources.cpp
	#/usr/local/Cellar/qt/5.15.0/bin/rcc -name resources customcompleter.qrc -o qrc_customcompleter.cpp
	#/usr/local/Cellar/qt/5.15.0/bin/rcc -name resources scripthelp.qrc -o qrc_scripthelp.cpp

HDRS = \
	$(PARAMS_HH) \
	Args.hh \
	Beam.hh \
	BoundaryPointEditor.hh \
	BoundaryPointEditorView.hh \
	BoundaryPointEditorModel.hh \
	BoundaryPointTool.hh \
	BoundaryView.hh \
	CloseEventFilter.hh \
	ColorBar.hh \
	ColorMap.hh \
	ColorTableManager.hh \
	FieldListView.hh \
	FieldRendererView.hh \
	FieldRendererController.hh \
	FlowLayout.hh \
	HawkEye.hh \
	HawkEyeLogger.hh \
	PaletteManager.hh \
        ParameterColorView.hh \
	Point.hh \
	PolygonUtils.hh \
        FieldColorController.hh \
	DataModel.hh \
	ScriptsDataModel.hh \
	ScriptsDataController.hh \
        DisplayFieldModel.hh \
        DisplayFieldController.hh \
        DisplayFieldView.hh \
	Params.hh \
	ParamFile.hh \
	PpiBeam.hh \
	PolarManager.hh \
	PolarWidget.hh \
	RayLoc.hh \
	RayLocationController.hh \
	RayLocationModel.hh \
	Reader.hh \
	ScaledLabel.hh \
	SiiPalette.hh \
	ScriptEditorController.hh \
	ScriptEditorModel.hh \
	ScriptEditorView.hh \
	ScriptEditorHelpItem.hh \
	ScriptEditorHelpModel.hh \
	SoloDefaultColorWrapper.hh \
	SoloFunctionsController.hh \
	SoloScriptTranslator.hh \
	SpreadSheetController.hh \
	SpreadSheetItem.hh \
	SpreadSheetModel.hh \
	SpreadSheetUtils.hh \
	SpreadSheetView.hh \
	StatusPanelController.hh \
	StatusPanelView.hh \
	SweepView.hh \
	SweepController.hh \
	SweepModel.hh \
	TimeNavView.hh \
	TimeNavController.hh \
	TimeNavModel.hh \
	TimeScaleWidget.hh \
	UndoRedoController.hh \
	UndoRedoModel.hh \
	WorldPlot.hh

NORM_SRCS = \
	$(PARAMS_CC) \
	AllocCheck.cc \
	Args.cc \
	Beam.cc \
	BoundaryPointEditorModel.cc \
	BoundaryView.cc \
	ColorMap.cc \
	ColorTableManager.cc \
	DataModel.cc \
	ScriptsDataModel.cc \
	ScriptsDataController.cc \
	DisplayField.cc \
	DisplayFieldModel.cc \
	DisplayFieldController.cc \
	FlowLayout.cc \
	HawkEye.cc \
	HawkEyeLogger.cc \
	Main.cc \
	PaletteManager.cc \
	ParamFile.cc \
	PpiBeam.cc \
	RayLocationController.cc \
	RayLocationModel.cc \
	Reader.cc \
	ScaledLabel.cc \
	ScriptEditorModel.cc \
	SiiPalette.cc \
	SoloDefaultColorWrapper.cc \
	SoloFunctionsModel.cc \
	SoloScriptTranslator.cc \
	SpreadSheetItem.cc \
	SpreadSheetModel.cc \
	SpreadSheetUtils.cc \
	SweepModel.cc \
	SweepController.cc \
	TimeNavModel.cc \
	TimeNavController.cc \
        UndoRedoController.cc \
        UndoRedoModel.cc \
	WorldPlot.cc 

MOC_SRCS = \
	FieldRendererView.cc \
	FieldListView.cc \
	FieldRendererController.cc \
	PolarManager.cc \
	PolarWidget.cc \
	TimeScaleWidget.cc \
	ParameterColorView.cc \
        FieldColorController.cc \
	BoundaryPointEditor.cc \
	BoundaryPointEditorView.cc \
	CloseEventFilter.cc \
	ColorBar.cc \
	ColorMapTemplates.cc \
	ClickableLabel.cc \
	ClickableLabelNamed.cc \
	DisplayFieldView.cc \
	ScriptEditorView.cc \
	ScriptEditorController.cc \
	ScriptEditorHelpItem.cc \
	ScriptEditorHelpModel.cc \
	TextEdit.cc \
	SoloFunctionsController.cc \
	SpreadSheetView.cc \
	SpreadSheetController.cc \
	SpreadSheetDelegate.cc \
	StatusPanelController.cc \
	StatusPanelView.cc \
	SweepView.cc \
	TimeNavView.cc \
	FunctionEditor.cc 

MOC_OUTPUT = \
	moc_BoundaryPointEditorView.cpp \
	moc_BoundaryPointEditor.cpp \
	moc_FieldListView.cpp \
	moc_FieldRendererView.cpp \
	moc_PolarManager.cpp \
	moc_PolarWidget.cpp \
	moc_TimeScaleWidget.cpp \
	moc_ParameterColorView.cpp \
        moc_FieldColorController.cpp \
	moc_CloseEventFilter.cpp \
	moc_ColorBar.cpp \
	moc_ColorMapTemplates.cpp \
	moc_ClickableLabel.cpp \
	moc_ClickableLabelNamed.cpp \
	moc_DisplayFieldView.cpp \
	moc_ScriptEditorView.cpp \
	moc_ScriptEditorController.cpp \
	moc_ScriptEditorHelpModel.cpp \
	moc_TextEdit.cpp \
	moc_SoloFunctionsController.cpp \
	moc_SpreadSheetView.cpp \
	moc_SpreadSheetController.cpp \
	moc_SpreadSheetDelegate.cpp \
	moc_StatusPanelController.cpp \
	moc_StatusPanelView.cpp \
	moc_SweepView.cpp \
	moc_FunctionEditor.cpp \
	moc_TimeNavView.cpp 

RESOURCES = \
	qrc_resources.cpp
	# qrc_customcompleter.cpp \
	# qrc_scripthelp.cpp 

CC_SRCS = $(NORM_SRCS) $(MOC_SRCS)
CPP_SRCS = $(MOC_OUTPUT) $(RESOURCES)

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

#	g++ -o SiiTest_Main SiiPalette.cc SiiTest_Main.cc SoloDefaultColorWrapper.cc PaletteManager.cc ColorTableManager.cc ColorMap.cc -I/Users/brenda/lrose/include -L/Users/brenda/lrose/lib -ltoolsa -ldataport -ltdrp -lz -lm -g


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
#	cp /usr/lib/libexpat.1.dylib  HawkEye.app/Contents/Frameworks/.
#	cp /usr/lib/libbz2.1.0.dylib  HawkEye.app/Contents/Frameworks/.
#	cp /usr/lib/libz.1.dylib      HawkEye.app/Contents/Frameworks/.
#	cp /usr/lib/libSystem.B.dylib HawkEye.app/Contents/Frameworks/.
	mkdir -p HawkEye.app/Contents/MacOS/../share
	cp -r ~/git/lrose-displays/color_scales HawkEye.app/Contents/MacOS/../share/.
# make the dmg  
# export MAC_QT_PATH="/usr/local/Cellar/qt/5.13.1/bin"
# must be set in the environment
# export PKG_CONFIG_PATH="/usr/local/opt/qt/lib/pkgconfig"
# /usr/local/Cellar/qt/5.15.0/bin/macdeployqt HawkEye.app -dmg -always-overwrite -codesign="${CERT_NAME}"
	/usr/local/Cellar/qt/5.15.0/bin/macdeployqt HawkEye.app -dmg -always-overwrite -sign-for-notarization="${CERT_NAME}"
# now build the folder for the dmg installer
# mkdir -p HawkEye_bkg_test
# cp ~/Desktop/arrow.pdf HawkEye_bkg_test/.background
# rsync -r HawkEye.app HawkEye_bkg_test/.
	hdiutil create -volname HawkEye_edit -srcfolder /Users/brenda/git/lrose-core/codebase/apps/radar/src/HawkEye/HawkEye.app -ov -format UDZO HawkEye_edit.dmg

# rm -r HawkEye_staging/HawkEye.app
# mkdir -p HawkEye_staging
# cp arrow.pdf HawkEye_staging/.background
# ln -s /Applications HawkEye_staging/Applications
# rsync -r HawkEye.app HawkEye_staging/.
# hdiutil create -volname HawkEye_Installer -srcfolder /Users/brenda/git/lrose-core/codebase/apps/radar/src/HawkEye/HawkEye.app  -ov -format UDZO HawkEye_Edit.dmg



DMG_BASE_DIR = /Users/brenda
hawkedit_app:
# pulled from circleci.yml
	echo "building HawkEdit.dmg"
# cd /private/tmp/workarea/$SOURCE_DIR/apps/radar/src/HawkEdit
# pwd
	make clean_all
#
# in dev area only ...
	make install  
# in circleci area only ...
#/usr/local/Cellar/qt/5.15.0/bin/qmake -o Makefile.qmake
#echo " ...  make from_qmake"
	make -f Makefile.qmake
#echo " ... after qmake"
# end of dev vs. circleci distinction 
#
	mkdir -p HawkEdit.app/Contents/Frameworks
	echo " ... after mkdir"
# cp /usr/local/Cellar/qt/5.15.0/plugins/platforms/libqcocoa.dylib HawkEdit.app/Contents/Plugins/platforms/.
#cp /usr/lib/libexpat.1.dylib HawkEdit.app/Contents/Frameworks/.
#cp /usr/lib/libbz2.1.0.dylib HawkEdit.app/Contents/Frameworks/.
#cp /usr/lib/libz.1.dylib     HawkEdit.app/Contents/Frameworks/.
#cp /usr/lib/libSystem.B.dylib HawkEdit.app/Contents/Frameworks/.
#
# add the color scales
#
	mkdir -p HawkEdit.app/Contents/MacOS/../share
# cp -r /private/tmp/workarea/$SOURCE_DIR/share/color_scales HawkEdit-Elle.app/Contents/MacOS/../share/.
	cp -r ~/git/lrose-displays/color_scales HawkEdit.app/Contents/MacOS/../share/.
#
# cp the HawkEdit executable to the HawkEdit.app area because make doesn't put it there.
#
# cp HawkEdit HawkEdit.app/Contents/MacOS/HawkEdit
#
	echo " ... before calling macdeployqt"
# macdeployqt HawkEdit.app -dmg -always-overwrite
# /usr/local/Cellar/qt/5.15.0/bin/macdeployqt HawkEdit.app -no-plugins -dmg -always-overwrite -codesign="${CERT_NAME}"
	/usr/local/Cellar/qt/5.15.0/bin/macdeployqt HawkEdit.app  -dmg -always-overwrite -codesign="${CERT_NAME}"
	echo " .. before calling hdiutil"
#hdiutil create -volname HawkEdit -srcfolder /Users/brenda/git/lrose-core/codebase/apps/radar/src/HawkEdit/HawkEdit.app -ov -format UDZO HawkEdit_Elle.dmg
## hdiutil create -volname HawkEdit-Elle_Installer -srcfolder /private/tmp/workarea/$SOURCE_DIR/apps/radar/src/HawkEdit/HawkEdit-Elle.app -ov -format UDZO HawkEdit-Elle.dmg
	echo " ... after dmg build"
# cp HawkEdit.dmg /private/tmp/workspace_macos/releases/lrose-cyclone
	echo " ... done  building HawkEdit"
#         hdiutil create -volname HawkEdit_Installer -srcfolder /Users/brenda/git/lrose-core/codebase/apps/radar/src/HawkEdit/HawkEdit.app  -ov -format UDZO HawkEdit_Elle.dmg



# DO NOT DELETE THIS LINE -- make depend depends on it.



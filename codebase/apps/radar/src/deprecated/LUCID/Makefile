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
# Makefile for LUCID program
#
# Mike Dixon, RAP, NCAR
# POBox 3000, Boulder, CO, 80307, USA
#
# May 2006
#
###########################################################################

include $(LROSE_CORE_DIR)/build/make_include/lrose_make_macros

TARGET_FILE = LUCID

# need to add $(NETCDF4_INCS) 
LOC_INCLUDES = $(QT_INCLUDES) $(HDRS) -I/usr/local/Cellar/qt/5.13.0/lib/QtCharts.framework/Headers $(NETCDF4_INCS)


# need to add -lRadx -lNcxx -ltoolsa -ldataport  $(NETCDF4_LIBS)
LOC_LIBS = $(QT_LIBS) $(NETCDF4_LIBS) \
	-ltdrp -lfftw3 -lbz2 -lz -lpthread -lm 

# need to add $(NETCDF4_LDFLAGS)
LOC_LDFLAGS = -L$(LROSE_INSTALL_DIR)/lib $(QT_LDFLAGS)  -F/usr/local/Cellar/qt/5.13.0/lib -framework QtCharts $(NETCDF4_LDFLAGS)

LOC_CPPC_CFLAGS = $(QT_CPPC_CFLAGS)

#$(PARAMS_HH) \
HDRS = \
	mainwindow.h \
	viewStatusDialog.h \
	viewVsection.h \
	viewGridConfigDialog.h \
	viewWindDialog.h \
	viewPlayerDock.h \
	viewZoomOptions.h \
	viewValuesDisplay.h \
	viewMainImage.h \
	Args.hh \
	Params.hh \
	contFieldDock.h \
    contGridConfigDialog.h \
    contMainImage.h \
    contMiscConfig.h \
    contOverlaysDialog.h \
    contPlayerDock.h \
    contStatusDialog.h \
    contToolBars.h \
    contValuesDisplay.h \
    contVsection.h \
    contWindDialog.h \
    contZoomOptions.h \
    viewMiscConfigDialog.h \
    viewOverlaysDialog.h

UI_HDRS = \
	ui_mainwindow.h 

#$(PARAMS_CC) \
NORM_SRCS = \
	mainwindow.cpp \
	main.cpp \
	viewStatusDialog.cpp \
	viewVsection.cpp \
	viewGridConfigDialog.cpp \
	viewWindDialog.cpp \
	viewPlayerDock.cpp \
	viewZoomOptions.cpp \
	viewValuesDisplay.cpp \
	viewMainImage.cpp \
	Args.cc \
	Params.cc \
	contFieldDock.cpp \
    contGridConfigDialog.cpp \
    contMainImage.cpp \
    contMiscConfig.cpp \
    contOverlaysDialog.cpp \
    contPlayerDock.cpp \
    contStatusDialog.cpp \
    contToolBars.cpp \
    contValuesDisplay.cpp \
    contVsection.cpp \
    contWindDialog.cpp \
    contZoomOptions.cpp \
    viewMiscConfigDialog.cpp \
    viewOverlaysDialog.cpp

MOC_SRCS = \
	mainwindow.cpp \
	viewStatusDialog.cpp \
	viewVsection.cpp \
	viewGridConfigDialog.cpp \
	viewWindDialog.cpp \
	viewPlayerDock.cpp \
	viewZoomOptions.cpp \
	viewValuesDisplay.cpp \
	viewMainImage.cpp \
	contFieldDock.cpp \
    contGridConfigDialog.cpp \
    contMainImage.cpp \
    contMiscConfig.cpp \
    contOverlaysDialog.cpp \
    contPlayerDock.cpp \
    contStatusDialog.cpp \
    contToolBars.cpp \
    contValuesDisplay.cpp \
    contVsection.cpp \
    contWindDialog.cpp \
    contZoomOptions.cpp \
    viewMiscConfigDialog.cpp \
    viewOverlaysDialog.cpp

MOC_OUTPUT = \
	moc_mainwindow.cpp \
	moc_viewStatusDialog.cpp \
	moc_viewVsection.cpp \
	moc_viewGridConfigDialog.cpp \
	moc_viewWindDialog.cpp \
	moc_viewPlayerDock.cpp \
	moc_viewZoomOptions.cpp \
	moc_viewValuesDisplay.cpp \
	moc_viewMainImage.cpp \
	qrc_res.cpp \
    moc_contMainImage.cpp \
    moc_contPlayerDock.cpp \
    moc_contStatusDialog.cpp \
    moc_contVsection.cpp \
    moc_viewMiscConfigDialog.cpp \
    moc_contOverlaysDialog.cpp \
    moc_viewOverlaysDialog.cpp


CC_SRCS = Args.cc Params.cc
CPP_SRCS = main.cpp $(NORM_SRCS) $(MOC_SRCS) $(MOC_OUTPUT)

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
# build Mac OSX application
#
LUCID_app:
# Make LUCID and the Makefile.qmake
	make 
# Make the bundle (LUCID.app tree structure)
	make from_qmake
# copy the system libraries to the .app tree structure
	mkdir -p LUCID.app/Contents/Frameworks
	cp /usr/lib/libexpat.1.dylib  LUCID.app/Contents/Frameworks/.
	cp /usr/lib/libbz2.1.0.dylib  LUCID.app/Contents/Frameworks/.
	cp /usr/lib/libz.1.dylib      LUCID.app/Contents/Frameworks/.
	cp /usr/lib/libSystem.B.dylib LUCID.app/Contents/Frameworks/.
	mkdir -p LUCID.app/Contents/MacOS/../share
	cp -r ~/git/lrose-displays/color_scales LUCID.app/Contents/MacOS/../share/.
# make the dmg  
	macdeployqt LUCID.app -dmg -always-overwrite -codesign="${CERT_NAME}"
# now build the folder for the dmg installer
	mkdir -p LUCID_bkg_test
	cp ~/Desktop/arrow.pdf LUCID_bkg_test/.background
	rsync -r LUCID.app LUCID_bkg_test/.
	hdiutil create -volname LUCID_Installer4 -srcfolder /Users/brenda/git/lrose-core/codebase/apps/radar/src/LUCID/LUCID_bkg_test -ov -format UDZO LUCID_test4.dmg


# rm -r LUCID_staging/LUCID.app
# mkdir -p LUCID_staging
# cp arrow.pdf LUCID_staging/.background
# ln -s /Applications LUCID_staging/Applications
# rsync -r LUCID.app LUCID_staging/.
	hdiutil create -volname LUCID_Installer -srcfolder /Users/brenda/git/lrose-core/codebase/apps/radar/src/LUCID/LUCID.app  -ov -format UDZO LUCID_Cyclone.dmg


# DO NOT DELETE THIS LINE -- make depend depends on it.


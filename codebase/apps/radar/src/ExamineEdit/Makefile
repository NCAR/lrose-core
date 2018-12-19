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

TARGET_FILE = ExamineEdit

LOC_INCLUDES = $(QT_INCLUDES) $(NETCDF4_INCS)

LOC_LIBS = -lSpdb -lradar -lrapformats -lFmq -lRadx \
	-ldsserver -ldidss -leuclid -lphysics \
	-lrapmath -ltoolsa -ldataport -ltdrp \
	$(QT_LIBS) $(NETCDF4_LIBS) \
	-lfftw3 -lbz2 -lz -lpthread -lm

LOC_LDFLAGS = -g -L$(LROSE_INSTALL_DIR)/lib $(QT_LDFLAGS) $(NETCDF4_LDFLAGS)

LOC_CPPC_CFLAGS = -g $(QT_CPPC_CFLAGS) -fno-limit-debug-info

HDRS = \
	moc_predefs.h \
	spreadsheetdelegate.hh \
	SpreadSheetUtils.hh \
	SpreadSheetModel.hh \
	SpreadSheetView.hh \
	SpreadSheetController.hh \
	spreadsheetitem.hh

NORM_SRCS = \
	Main.cc \
	spreadsheetdelegate.cc \
	SpreadSheetUtils.cc \
	SpreadSheetModel.cc \
	SpreadSheetView.cc \
	SpreadSheetController.cc \
	spreadsheetitem.cc  \
	qrc_spreadsheet.cc

MOC_SRCS = \
	SpreadSheetView.cc \
	spreadsheetdelegate.cc

MOC_OUTPUT = \
	moc_SpreadSheetView.cpp \
	moc_spreadsheetdelegate.cpp

CC_SRCS = $(NORM_SRCS)
#  $(MOC_SRCS)
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

#
# build Mac OSX application
#
examine_edit_app:
	rm -rf ExamineEdit_bkg_test
	rm -rf ExamineEdit.app
	# Make ExamineEdit  and the Makefile.qmake
	make
	# Make the bundle (ExamineEdit.app tree structure)
	make from_qmake
	# copy the system libraries to the .app tree structure
	mkdir ExamineEdit.app/Contents/Frameworks
	cp /usr/lib/libexpat.1.dylib  ExamineEdit.app/Contents/Frameworks/.
	cp /usr/lib/libbz2.1.0.dylib  ExamineEdit.app/Contents/Frameworks/.
	cp /usr/lib/libz.1.dylib      ExamineEdit.app/Contents/Frameworks/.
	cp /usr/lib/libSystem.B.dylib ExamineEdit.app/Contents/Frameworks/.
	# mkdir ExamineEdit.app/Contents/MacOS/../share
	# cp -r ~/git/lrose-displays/color_scales ExamineEdit.app/Contents/MacOS/../share/.
	# make the dmg
	macdeployqt ExamineEdit.app -always-overwrite
	# now build the folder for the dmg installer
	mkdir ExamineEdit_bkg_test
	cp ~/Desktop/arrow.pdf ExamineEdit_bkg_test/.background
	cp -R ExamineEdit.app ExamineEdit_bkg_test
	hdiutil create -volname ExamineEdit_Installer -srcfolder /Users/brenda/git/lrose-core/codebase/apps/radar/src/ExamineEdit/ExamineEdit_bkg_test -ov -format UDZO ExamineEdit_test.dmg


# DO NOT DELETE THIS LINE -- make depend depends on it.


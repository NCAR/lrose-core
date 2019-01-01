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
# Makefile for RadxPrint program
#
# Mike Dixon, RAP, NCAR, Boulder, CO, 80307, USA
#
# Jan 2010
#
###########################################################################

include $(RAP_MAKE_INC_DIR)/rap_make_macros

TARGET_FILE = RadxPrint

LOC_INCLUDES = $(NETCDF4_INCS)

LOC_LIBS = -lMdv -lRadx -lNcxx -ldsserver -ldidss \
	-lrapformats -leuclid -ltoolsa -ltdrp \
	-ldataport $(NETCDF4_LIBS) -lbz2 -lz

LOC_LDFLAGS = $(NETCDF4_LDFLAGS)

LOC_CFLAGS =

HDRS = \
	Params.hh \
	Args.hh \
	RadxPrint.hh

CPPC_SRCS = \
	Params.cc \
	Args.cc \
	Main.cc \
	RadxPrint.cc

#
# tdrp macros
#

include $(RAP_MAKE_INC_DIR)/rap_make_tdrp_macros

#
# C++ targets
#

include $(RAP_MAKE_INC_DIR)/rap_make_c++_targets

#
# tdrp targets
#

include $(RAP_MAKE_INC_DIR)/rap_make_tdrp_c++_targets

#
# QT C++ targets
#

include $(RAP_MAKE_INC_DIR)/rap_make_qt_targets

#
# local targets
#

depend: depend_generic

#
# build Mac OSX application
#
RadxPrint_app:
	@echo $(HOME)
	# Make HawkEye and the Makefile.qmake
	make 
	# Make the bundle (HawkEye.app tree structure)
	make from_qmake
	# make -f Makefile.qmake
	# copy the system libraries to the .app tree structure
	mkdir -p RadxPrint.app/Contents/Frameworks
	cp /usr/lib/libexpat.1.dylib  RadxPrint.app/Contents/Frameworks/.
	cp /usr/lib/libbz2.1.0.dylib  RadxPrint.app/Contents/Frameworks/.
	cp /usr/lib/libz.1.dylib      RadxPrint.app/Contents/Frameworks/.
	cp /usr/lib/libSystem.B.dylib RadxPrint.app/Contents/Frameworks/.
	# mkdir -p RadxPrint.app/Contents/MacOS/../share
	# cp -r ~/git/lrose-displays/color_scales RadxPrint.app/Contents/MacOS/../share/.
	# make the dmg  
	macdeployqt RadxPrint.app -dmg -always-overwrite -codesign="${CERT_NAME}"
	# now build the folder for the dmg installer
	mkdir -p RadxPrint_bkg_test
	cp ~/Desktop/arrow.pdf RadxPrint_bkg_test/.background
	rsync -r RadxPrint.app RadxPrint_bkg_test/.
	hdiutil create -volname RadxPrint_Installer -srcfolder /Users/brenda/git/lrose-core/codebase/apps/Radx/src/RadxPrint/RadxPrint_bkg_test -ov -format UDZO RadxPrint_test.dmg

# DO NOT DELETE THIS LINE -- make depend depends on it.


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
# Makefile for HawkEyeGl program
#
# Mike Dixon, RAP, NCAR
# POBox 3000, Boulder, CO, 80307, USA
#
# May 2006
#
###########################################################################

include $(RAP_MAKE_INC_DIR)/rap_make_macros

TARGET_FILE = HawkEyeGl

LOC_INCLUDES = $(QT4_INCLUDES) \
       -I/usr/include/qt4/QtOpenGL \
       -I/usr/lib/qt4/include/QtOpenGL

LOC_CPPC_CFLAGS =

LOC_LDFLAGS = $(QT4_LDFLAGS)
LOC_LIBS = \
          -ldsdata -lradar -lFmq -lSpdb -lMdv  -ldsserver -ldidss \
          -lRadx -lNcxx -lrapformats -leuclid -lrapmath \
          -lphysics -ltoolsa -ldataport -ltdrp \
          $(NETCDF4_LIBS) \
          -lfftw3 -lbz2 -lz -lpthread

HDRS = \
	$(PARAMS_HH) \
	Args.hh \
	ColorBar.hh \
	ColorMap.hh \
	HawkEyeGl.hh \
	Params.hh \
	PPI.hh \
	Reader.hh \
	ScaledLabel.hh

NORM_SRCS = \
	$(PARAMS_CC) \
	Args.cc \
	ColorMap.cc \
	Main.cc \
	ScaledLabel.cc \

MOC_SRCS = \
	ColorBar.cc \
	HawkEyeGl.cc \
	PPI.cc \
	Reader.cc

MOC_OUTPUT = \
	moc_ColorBar.cpp \
	moc_HawkEyeGl.cpp \
	moc_PPI.cpp \
	moc_Reader.cpp

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

# DO NOT DELETE THIS LINE -- make depend depends on it.



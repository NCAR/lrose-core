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
# Makefile for Sprite program
#
# Mike Dixon, EOL, NCAR
# POBox 3000, Boulder, CO, 80307, USA
#
# March 2019
#
###########################################################################

include $(RAP_MAKE_INC_DIR)/rap_make_macros

TARGET_FILE = Sprite

LOC_INCLUDES = $(QT_INCLUDES) $(NETCDF4_INCS)

LOC_LIBS = -lradar -lrapformats -lFmq \
	-ldsserver -ldidss -leuclid -lphysics \
	-lrapmath -ltoolsa -ldataport -ltdrp \
	$(QT_LIBS) $(NETCDF4_LIBS) \
	-lfftw3 -lbz2 -lz -lpthread -lm

LOC_LDFLAGS = -L$(LROSE_INSTALL_DIR)/lib $(QT_LDFLAGS) $(NETCDF4_LDFLAGS)

LOC_CPPC_CFLAGS = $(QT_CPPC_CFLAGS)

HDRS = \
	$(PARAMS_HH) \
	Args.hh \
	AScopeManager.hh \
	ColorMap.hh \
	Sprite.hh \
	ScaledLabel.hh \
	WorldPlot.hh

NORM_SRCS = \
	$(PARAMS_CC) \
	Args.cc \
	ColorMap.cc \
	Sprite.cc \
	Main.cc \
	ScaledLabel.cc \
	WorldPlot.cc

MOC_SRCS = \
	AScopeManager.cc \
	AScopeWidget.cc \
	TimeScaleWidget.cc \

MOC_OUTPUT = \
	moc_AScopeManager.cpp \
	moc_AScopeWidget.cpp \
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

# DO NOT DELETE THIS LINE -- make depend depends on it.


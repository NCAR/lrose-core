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

include $(LROSE_CORE_DIR)/build/make_include/lrose_make_macros

TARGET_FILE = Sprite

LOC_INCLUDES = $(QT_INCLUDES) $(NETCDF4_INCS)

LOC_LIBS = \
          -ldsdata -lradar -lFmq -lSpdb -lMdv  -ldsserver -ldidss \
          -lRadx -lNcxx -lrapformats -leuclid -lrapmath \
          -lphysics -ltoolsa -ldataport -ltdrp \
          $(QT_LIBS) $(QJSENGINE) $(NETCDF4_LIBS) \
          -lfftw3 -lbz2 -lz -lpthread

LOC_LDFLAGS = -L$(LROSE_INSTALL_DIR)/lib $(QT_LDFLAGS) $(NETCDF4_LDFLAGS)

LOC_CPPC_CFLAGS = $(QT_CPPC_CFLAGS)

HDRS = \
	$(PARAMS_HH) \
	Args.hh \
	AscopePlot.hh \
	Beam.hh \
	BeamMgr.hh \
	ColorMap.hh \
	IqPlot.hh \
	RadarSpectra.hh \
	ScaledLabel.hh \
	SpectraMgr.hh \
	SpectraWidget.hh \
	Sprite.hh \
	TsReader.hh \
	WaterfallPlot.hh \
	WorldPlot.hh

NORM_SRCS = \
	$(PARAMS_CC) \
	Args.cc \
	AscopePlot.cc \
	Beam.cc \
	BeamMgr.cc \
	ColorMap.cc \
	IqPlot.cc \
	Main.cc \
	RadarSpectra.cc \
	ScaledLabel.cc \
	Sprite.cc \
	TsReader.cc \
	WaterfallPlot.cc \
	WorldPlot.cc

MOC_SRCS = \
	SpectraMgr.cc \
	SpectraWidget.cc

MOC_OUTPUT = \
	moc_SpectraMgr.cpp \
	moc_SpectraWidget.cpp

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

# DO NOT DELETE THIS LINE -- make depend depends on it.


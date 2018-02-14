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
# Makefile for Titan program
#
# Mike Dixon, RAP, NCAR, Boulder, CO, 80307, USA
#
# Nov 1998
#
###########################################################################

include $(RAP_MAKE_INC_DIR)/rap_make_macros

TARGET_FILE = Titan

LOC_INCLUDES = $(NETCDF4_INCS)

LOC_LIBS = -lradar -lMdv -lRadx -lNcxx -lSpdb -ldsserver \
	-ldidss -ltitan -lrapmath -leuclid \
	-lrapformats -lphysics -ltoolsa -ldataport \
	-ltdrp $(NETCDF4_LIBS) -lbz2 -lz \
	-lpthread

LOC_LDFLAGS = $(NETCDF4_LDFLAGS)

LOC_CFLAGS =

HDRS = \
	$(PARAMS_HH) \
	Args.hh \
	Area.hh \
	Boundary.hh \
	Clumping.hh \
	ConvectionFinder.hh \
	DataTimes.hh \
	DualThresh.hh \
	FileLock.hh \
	GridClump.hh \
	Identify.hh \
	InputMdv.hh \
	OutputMdv.hh \
	Props.hh \
	Sounding.hh \
	StormIdent.hh \
	StormTrack.hh \
	TitanDriver.hh \
	TrConsolidate.hh \
	TrForecast.hh \
	TrOverlaps.hh \
	TrStorm.hh \
	TrTrack.hh \
	Verify.hh \
	Worker.hh

CPPC_SRCS = \
	$(PARAMS_CC) \
	Args.cc \
	Area.cc \
	Boundary.cc \
	Clumping.cc \
	ConvectionFinder.cc \
	DataTimes.cc \
	DualThresh.cc \
	FileLock.cc \
	GridClump.cc \
	Identify.cc \
	InputMdv.cc \
	Main.cc \
	OutputMdv.cc \
	Props.cc \
	Sounding.cc \
	StormIdent.cc \
	StormTrack.cc \
	TitanDriver.cc \
	TrConsolidate.cc \
	TrForecast.cc \
	TrMatch.cc \
	TrOverlaps.cc \
	TrStorm.cc \
	TrTrack.cc \
	TrUpdate.cc \
	Verify.cc \
	Worker.cc

#
# tdrp macros
#

include $(RAP_MAKE_INC_DIR)/rap_make_tdrp_macros

#
# standard C++ targets
#

include $(RAP_MAKE_INC_DIR)/rap_make_c++_targets

#
# tdrp targets
#

include $(RAP_MAKE_INC_DIR)/rap_make_tdrp_c++_targets

#
# local targets
#

# DO NOT DELETE THIS LINE -- make depend depends on it.


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
# Makefile for StormModel program
#
# Mike Dixon, RAP, NCAR, Boulder, Co, USA, 80307
#
# February 1998
#
###########################################################################

include $(LROSE_CORE_DIR)/build/make_include/lrose_make_macros

TARGET_FILE = StormModel

LOC_INCLUDES = $(NETCDF4_INCS)

LOC_LIBS = \
	-lMdv -lrapformats -ldsserver -ldidss \
	-leuclid -lrapmath -ltoolsa -ldataport \
	-ltdrp -lRadx -lNcxx -lphysics \
	$(NETCDF4_LIBS) -lpthread -lbz2 -lz \
	-lm

LOC_LDFLAGS = $(NETCDF4_LDFLAGS)

LOC_CFLAGS =

HDRS = \
	Params.hh \
	Area.hh \
	Args.hh \
	Duration.hh \
	Generate.hh \
	GenFromCdf.hh \
	MidPoint.hh \
	StartTime.hh \
	StormModel.hh \
	Velocity.hh

CPPC_SRCS = \
	Params.cc \
	Area.cc \
	Args.cc \
	Duration.cc \
	Generate.cc \
	GenFromCdf.cc \
	Main.cc \
	MidPoint.cc \
	StartTime.cc \
	StormModel.cc \
	Velocity.cc

#
# tdrp support
#

include $(LROSE_CORE_DIR)/build/make_include/lrose_make_tdrp_macros

#
# general targets
#

include $(LROSE_CORE_DIR)/build/make_include/lrose_make_c++_targets

#
# local targets
#

include $(LROSE_CORE_DIR)/build/make_include/lrose_make_tdrp_c++_targets

# DO NOT DELETE THIS LINE -- make depend depends on it.



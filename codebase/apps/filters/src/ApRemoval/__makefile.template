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
# Makefile for ApRemoval program
#
# Jaimi Yee, RAP, NCAR, Boulder, CO, 80307, USA
#
# April 2002
#
###########################################################################
include $(LROSE_CORE_DIR)/build/make_include/lrose_make_macros

TARGET_FILE = ApRemoval
MODULE_TYPE = progcpp


LOC_INCLUDES = $(NETCDF4_INCS)

LOC_LIBS = \
	-lMdv -lFmq -lrapformats -ldsserver \
	-ldidss -leuclid -lrapmath -ltoolsa \
	-ldataport -ltdrp -lRadx -lNcxx \
	-lphysics $(NETCDF4_LIBS) -lbz2 -lz \
	-lpthread -lm

LOC_LDFLAGS = $(NETCDF4_LDFLAGS)

LOC_CPPC_CFLAGS =

HDRS = \
	Params.hh \
	ApRemoval.hh \
	DataMgr.hh \
	Feature.hh \
	Filter.hh \
	FilterBeamInfo.hh \
	FilterList.hh \
	GTFilter.hh \
	InterestFunction.hh \
	LTFilter.hh \
	RadarMsg.hh \
	RadarTilt.hh \
	TerrainMask.hh

CPPC_SRCS = \
	Params.cc \
	ApRemoval.cc \
	DataMgr.cc \
	Feature.cc \
	Filter.cc \
	FilterBeamInfo.cc \
	FilterList.cc \
	GTFilter.cc \
	InterestFunction.cc \
	LTFilter.cc \
	RadarMsg.cc \
	RadarTilt.cc \
	TerrainMask.cc \
	dieGracefully.cc \
	main.cc

#
# tdrp macros
#

include $(LROSE_CORE_DIR)/build/make_include/lrose_make_tdrp_macros

#
# standard C++ targets
#

include $(LROSE_CORE_DIR)/build/make_include/lrose_make_c++_targets

#
# tdrp targets
#

include $(LROSE_CORE_DIR)/build/make_include/lrose_make_tdrp_c++_targets

#
# local targets
#

# DO NOT DELETE THIS LINE -- make depend depends on it.



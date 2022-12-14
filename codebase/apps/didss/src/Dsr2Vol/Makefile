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
# Makefile for Dsr2Vol program
#
# Mike Dixon, RAP, NCAR
# POBox 3000, Boulder, CO, 80307, USA
#
# July 2001
#
###########################################################################

include $(LROSE_CORE_DIR)/build/make_include/lrose_make_macros

TARGET_FILE = Dsr2Vol

LOC_INCLUDES = $(NETCDF4_INCS) -I/usr/local/udunits2/include

LOC_LIBS = \
	-lMdv -lFmq -lrapformats -ldsserver \
	-ldidss -leuclid -lrapmath -ltoolsa \
	-ldataport -ltdrp -lRadx -lNcxx \
	-lphysics $(NETCDF4_LIBS) -lbz2 -lz \
	-lpthread

LOC_LDFLAGS = $(NETCDF4_LDFLAGS) -L/usr/local/udunits2/lib

HDRS = \
	$(PARAMS_HH) \
	Antenna.hh \
	Args.hh \
	Beam.hh \
	BeamGeom.hh \
	BeamGeomMgr.hh \
	CartTransform.hh \
	Dsr2Vol.hh \
	FieldInfo.hh \
	OutputMdv.hh \
	PlanTransform.hh \
	PolarTransform.hh \
	PpiMgr.hh \
	PpiTransform.hh \
	RhiTransform.hh \
	Transform.hh

CPPC_SRCS = \
	$(PARAMS_CC) \
	Antenna.cc \
	Args.cc \
	Beam.cc \
	BeamGeom.cc \
	BeamGeomMgr.cc \
	CartTransform.cc \
	Dsr2Vol.cc \
	FieldInfo.cc \
	Main.cc \
	OutputMdv.cc \
	PlanTransform.cc \
	PolarTransform.cc \
	PpiMgr.cc \
	PpiTransform.cc \
	RhiTransform.cc \
	Transform.cc

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



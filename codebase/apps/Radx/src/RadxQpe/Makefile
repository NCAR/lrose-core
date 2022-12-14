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
# makefile for RadxQpe
#
#
###########################################################################

include $(LROSE_CORE_DIR)/build/make_include/lrose_make_macros

TARGET_FILE = RadxQpe

LOC_INCLUDES = $(NETCDF4_INCS) -I.

LOC_LIBS = \
	-ldsdata -lradar -lMdv -lSpdb \
	-lFmq -lrapformats -ldsserver -ldidss \
	-leuclid -lrapmath -ltoolsa -ldataport \
	-ltdrp -lRadx -lNcxx -lphysics \
	$(NETCDF4_LIBS) -lfftw3 -lbz2 -lz \
	-lpthread

LOC_LDFLAGS = $(NETCDF4_LDFLAGS) $(HDF5_LDFLAGS) -L.

LOC_INC_DIR = .
MY_CPPCHECKFLAGS = -E Params.cc -E Params.hh -AddStyleFAA -exclude-rules="4.16,4.09,4.14,4.12"

LOC_CPPC_CFLAGS = -std=c++0x


HDRS = \
	$(PARAMS_HH) \
	Args.hh \
	AzMapper.hh \
	BeamBlock.hh \
	Data.hh \
	Field.hh \
	GateMapper.hh \
	Geom.hh \
	InputData.hh \
	Interp.hh \
	Out.hh \
	OutputData.hh \
	PpiInterp.hh \
	RadxQpe.hh \
	RadxQpeMgr.hh \
	Sweep.hh \
	Parms.hh \
	VertPrecipData.hh \
	VertData.hh

CPPC_SRCS = \
	$(PARAMS_CC) \
	Args.cc \
	AzMapper.cc \
	BeamBlock.cc \
	Data.cc \
	Field.cc \
	GateMapper.cc \
	Geom.cc \
	InputData.cc \
	Interp.cc \
	PpiInterpInfo.cc \
	PpiInterp.cc \
	Out.cc \
	OutputData.cc \
	OutputMdv.cc \
	RadxQpe.cc \
	RadxQpeMgr.cc \
	Sweep.cc \
	Main.cc \
	Parms.cc \
	VertPrecipData.cc \
	VertData.cc

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



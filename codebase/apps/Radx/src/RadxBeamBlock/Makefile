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
# makefile for RadxBeamBlock program
#
# Mike Dixon, RAP, NCAR
# POBox 3000, Boulder, CO, 80307, USA
#
# Feb 2011
#
###########################################################################

include $(LROSE_CORE_DIR)/build/make_include/lrose_make_macros

TARGET_FILE = RadxBeamBlock

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
MY_CPPCHECKFLAGS = -E Params.cc -E Params.hh -i ./BeamBlock -AddStyleFAA -exclude-rules="4.16,4.09,4.14,4.12"

LOC_CPPC_CFLAGS = -std=c++11


HDRS = \
	$(PARAMS_HH) \
	Args.hh \
	GateHandler.hh \
	RayHandler.hh \
	ScanHandler.hh \
	VolHandler.hh \
	DigitalElevationHandler.hh \
	RadxBeamBlock.hh \
	RadxBeamBlockMgr.hh \
	Parms.hh

CPPC_SRCS = \
	$(PARAMS_CC) \
	Args.cc \
	GateHandler.cc \
	RayHandler.cc \
	ScanHandler.cc \
	VolHandler.cc \
	DigitalElevationHandler.cc \
	RadxBeamBlock.cc \
	RadxBeamBlockMgr.cc \
	Main.cc \
	Parms.cc \
        angle.cc \
	beam_power.cc \
	beam_propagation.cc \
	digital_elevation.cc \
	field.cc \
	latlon.cc \
	spheroid.cc \
	string_utils.cc  \
	trace.cc \
	xml.cc

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



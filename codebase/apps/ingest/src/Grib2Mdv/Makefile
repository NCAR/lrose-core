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
# Makefile for Grib2Mdv program
#
# RAP, NCAR, Boulder, CO, 80307, USA
#
# Author:  Carl Drews
# March 2004
#
###########################################################################

include $(LROSE_CORE_DIR)/build/make_include/lrose_make_macros

TARGET_FILE = Grib2Mdv
MODULE_TYPE = progcpp


LOC_INCLUDES = $(NETCDF4_INCS)

LOC_LIBS = -lgrib -lMdv -lRadx -lNcxx -ldsserver \
	-ldidss -lrapformats -lphysics -leuclid \
	-ltoolsa -ldataport -ltdrp -lrapmath \
	$(NETCDF4_LIBS) -lbz2 -lz -lpthread \
	-lm

LOC_LDFLAGS = $(NETCDF4_LDFLAGS)

LOC_CPPC_CFLAGS =

HDRS = \
	$(PARAMS_HH) \
	Args.hh \
	DataMgr.hh \
	FilelistInputStrategy.hh \
	GribMgr.hh \
	GribField.hh \
	Ingester.hh \
	InputStrategy.hh \
	LdataInputStrategy.hh \
	OutputFile.hh \
	RealtimeDirInputStrategy.hh \
	NewFilesInputStrategy.hh \
	RucGribMgr.hh \
	AvnGribMgr.hh \
	WafsGribMgr.hh \
	Quasi.hh \
	AfwaGribMgr.hh \
	DtraGribMgr.hh \
	Grib2Mdv.hh


CPPC_SRCS = \
	$(PARAMS_CC) \
	Args.cc \
	DataMgr.cc \
	FilelistInputStrategy.cc \
	GribMgr.cc \
	GribField.cc \
	Ingester.cc \
	InputStrategy.cc \
	LdataInputStrategy.cc \
	RealtimeDirInputStrategy.cc \
	NewFilesInputStrategy.cc \
	OutputFile.cc \
	RucGribMgr.cc \
	AvnGribMgr.cc \
	WafsGribMgr.cc \
	Quasi.cc \
	AfwaGribMgr.cc \
	DtraGribMgr.cc \
	Grib2Mdv.cc \
	main.cc

include $(LROSE_CORE_DIR)/build/make_include/lrose_make_tdrp_macros

#
# standard targets
#

include $(LROSE_CORE_DIR)/build/make_include/lrose_make_c++_targets

#
# tdrp
#

include $(LROSE_CORE_DIR)/build/make_include/lrose_make_tdrp_c++_targets

#
# local targets
#

depend: depend_generic

# DO NOT DELETE THIS LINE -- make depend depends on it.


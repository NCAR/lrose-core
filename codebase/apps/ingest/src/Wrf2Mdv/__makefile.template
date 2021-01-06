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
# Makefile for MM5Ingest program
#
# Mike Dixon, RAP, NCAR, Boulder, CO, 80307, USA
#
# Sept 1998
#
###########################################################################

include $(LROSE_CORE_DIR)/build/make_include/lrose_make_macros

TARGET_FILE = Wrf2Mdv

LOC_INCLUDES = $(NETCDF4_INCS)

LOC_LIBS = -lMdv -lRadx -lNcxx -ldsserver -lrapformats \
	-ldidss -lphysics -leuclid -ltoolsa \
	-ldataport -ltdrp $(NETCDF4_LIBS) -lbz2 \
	-lz -lpthread -lm

LOC_LDFLAGS = $(NETCDF4_LDFLAGS)

LOC_CFLAGS = 


#
# To compile on smac-c2-int2 with all-PGI need to make the following changes:
#
##LOC_INCLUDES = -I/opt/netcdf/include
##LOC_LDFLAGS = -L/opt/netcdf/lib  -fPIC
##LOC_LIBS = -lMdv -lRadx -lNcxx -ldsserver -lrapformats -ldidss -lphysics \
##		-ltoolsa -ldataport -ltdrp -lm /opt/netcdf/lib/libnetcdf_c++.a  -lnetcdf -lpthread


MODULE_TYPE = progcpp

HDRS = \
	$(PARAMS_HH) \
	Args.hh \
	InputPath.hh \
	Wrf2Mdv.hh \
	OutputFile.hh \
	WRFData.hh \
	WRFGrid.hh \
	PresInterp.hh \
	VisCalc.hh

CPPC_SRCS = \
	$(PARAMS_CC) \
	Args.cc \
	InputPath.cc \
	Main.cc \
	Wrf2Mdv.cc \
	OutputFile.cc \
	WRFData.cc \
	WRFGrid.cc \
	PresInterp.cc \
	VisCalc.cc

#
# tdrp macros
#

include $(LROSE_CORE_DIR)/build/make_include/lrose_make_tdrp_macros

#
# standard C++ targets
#

#include $(LROSE_CORE_DIR)/build/make_include/lrose_make_opt_c++_targets
include $(LROSE_CORE_DIR)/build/make_include/lrose_make_c++_targets

#
# tdrp targets
#

include $(LROSE_CORE_DIR)/build/make_include/lrose_make_tdrp_c++_targets

#
# local targets
#

depend: depend_generic

# DO NOT DELETE THIS LINE -- make depend depends on it.


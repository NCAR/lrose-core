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
# Makefile for REPoH
#
###########################################################################

include $(RAP_MAKE_INC_DIR)/rap_make_macros

MODULE_TYPE=progcpp
TARGET_FILE=REPoH
LOC_INCLUDES = -I. $(NETCDF4_INCS)

LOC_LIBS = -lFiltAlg -ldsdata -lSpdb -lMdv \
	-lRadx -ldsserver -ldidss -lrapformats \
	-leuclid -lrapmath -ltoolsa -ldataport \
	-ltdrp $(NETCDF4_LIBS) -lbz2 -lz \
	-lpthread -lm

LOC_LDFLAGS = $(NETCDF4_LDFLAGS)

LOC_CFLAGS = 
LOC_CPPC_CFLAGS = 

MY_CPPCHECKFLAGS = -E Params.cc -E Params.hh -AddStyleFAA -exclude-rules="4.16,4.09,4.14,4.12"
LOC_INC_DIR = .

HDRS = 

CPPC_SRCS = \
	$(PARAMS_CC) \
	AsciiOutput.cc \
	CloudEdge.cc \
	CloudGaps.cc \
	CloudGap.cc \
	ClumpAssociate.cc \
	ClumpAssociate1.cc \
	Main.cc \
	Parms.cc \
	MyFiltCreate.cc \
	FiltHumidity.cc \
	HumidityAlg.cc \
	Kernel.cc \
	KernelDbzDiffFilter.cc \
	KernelGrids.cc \
	Kernels.cc \
	KernelPair.cc \
	KernelTemplate.cc


#
# tdrp support
#
include $(RAP_MAKE_INC_DIR)/rap_make_tdrp_macros

#
# Standard c++ targets
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


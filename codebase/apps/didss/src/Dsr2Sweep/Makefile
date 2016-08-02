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
# Makefile for Dsr2Sweep program
#
# Nancy Rehak, RAP, NCAR
# POBox 3000, Boulder, CO, 80307, USA
#
# April 2006
#
###########################################################################

# NOTE ON FORAY library
#
# Install the foray includes in $(RAP_INC_DIR)/foray
# Install the foray .a files in $(RAP_LIB_DIR)/foray

include $(RAP_MAKE_INC_DIR)/rap_make_macros

TARGET_FILE = Dsr2Sweep

# Use the following for linking against lib forayRal, which is
# built using make, with HDF5 support

LOC_INCLUDES = $(NETCDF4_INCS)
LOC_CPPC_CFLAGS =
LOC_LDFLAGS = $(NETCDF4_LDFLAGS)
LOC_CFLAGS =

LOC_LIBS = \
	-lFmq -ldsserver -lrapformats -ldidss \
	-ltoolsa -ldataport -ltdrp -lforayRal \
	-lnetcdf -lnetcdf_c++ -lhdf5_hl -lhdf5 \
	-ludunits2 -lexpat -lbz2 -lz

# Use the following for linking against lib forayRal, which is
# built using make, without HDF5 support

#LOC_INCLUDES = $(NETCDF_INCS)
#LOC_CPPC_CFLAGS =
#LOC_LDFLAGS = $(NETCDF_LDFLAGS)
#LOC_CFLAGS =

#LOC_LIBS = -lFmq -ldsserver -lrapformats -ldidss -ltoolsa -lz -ldataport \
#		-ltdrp -lforayRal -lnetcdf -lnetcdf_c++

# Use the following for linking against Foray built using scons
#LOC_LIBS = -lFmq -ldsserver -lrapformats -ldidss \
#	-ltoolsa -lz -ldataport -ltdrp \
#	-ldorade -lncradar -lrayfile -lforayutil \
#	-lnetcdf -lnetcdf_c++ -lpthread

HDRS = \
	$(PARAMS_HH) \
	Args.hh \
	DoradeFileHandler.hh \
	Dsr2Sweep.hh \
	EosDetector.hh\
	FieldInfo.hh \
	FileHandler.hh \
	NcFileHandler.hh \
	Beam.hh \
	ScanStrategyEosDetector.hh \
	SweepNumEosDetector.hh

CPPC_SRCS = \
	$(PARAMS_CC) \
	Args.cc \
	DoradeFileHandler.cc \
	Dsr2Sweep.cc \
	EosDetector.cc \
	FieldInfo.cc \
	FileHandler.cc \
	Main.cc \
	NcFileHandler.cc \
	Beam.cc \
	ScanStrategyEosDetector.cc \
	SweepNumEosDetector.cc

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



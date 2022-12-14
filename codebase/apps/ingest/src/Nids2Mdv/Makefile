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
# Makefile for Nids2Mdv program
#
# Paddy McCarthy, RAP, NCAR
# POBox 3000, Boulder, CO, 80307, USA
#
# After NidsRadial2Mdv and NidsRaster2Mdv by Mike Dixon
# March 2001
#
###########################################################################

include $(LROSE_CORE_DIR)/build/make_include/lrose_make_macros

TARGET_FILE = Nids2Mdv

LOC_INCLUDES = $(NETCDF4_INCS)

LOC_LIBS = \
	-lMdv -lFmq -lrapformats -ldsserver \
	-ldidss -leuclid -lrapmath -ltoolsa \
	-ldataport -ltdrp -lRadx -lNcxx \
	-lphysics -lpthread $(NETCDF4_LIBS) -lbz2 \
	-lz -lm

LOC_LDFLAGS = $(NETCDF4_LDFLAGS)

LOC_CFLAGS =

HDRS = \
	$(PARAMS_HH) \
	Args.hh \
	Nids2Mdv.hh \
	OutputGeometry.hh \
	OutputMdv.hh \
	InputFetcher.hh \
	ArchiveInputFetcher.hh \
	RealtimeInputFetcher.hh \
	Remap.hh \
	RemapRast.hh \
	RemapRadial.hh

CPPC_SRCS = \
	$(PARAMS_CC) \
	Args.cc \
	Main.cc \
	Nids2Mdv.cc \
	OutputGeometry.cc \
	OutputMdv.cc \
	InputFetcher.cc \
	ArchiveInputFetcher.cc \
	RealtimeInputFetcher.cc \
	Remap.cc \
	RemapRast.cc \
	RemapRadial.cc

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



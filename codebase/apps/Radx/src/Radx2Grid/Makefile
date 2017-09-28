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
# Makefile for Radx2Grid program
#
# Mike Dixon, EOL, NCAR, Boulder, CO, 80307, USA
#
# March 2012
#
###########################################################################

include $(RAP_MAKE_INC_DIR)/rap_make_macros

TARGET_FILE = Radx2Grid

LOC_INCLUDES = $(NETCDF4_INCS)

LOC_LIBS = -lradar -lMdv -lRadx -lNcxx -ldsserver \
	-ldidss -lrapformats -lrapmath -lkd \
	-leuclid -ltoolsa -ltdrp -ldataport \
	$(NETCDF4_LIBS) -lbz2 -lz -lpthread

LOC_LDFLAGS = $(NETCDF4_LDFLAGS)

LOC_CFLAGS =

HDRS = \
	Params.hh \
	Args.hh \
	CartInterp.hh \
	Interp.hh \
	OutputMdv.hh \
	PpiInterp.hh \
	PolarInterp.hh \
	Radx2Grid.hh \
	ReorderInterp.hh \
	SatInterp.hh

CPPC_SRCS = \
	Params.cc \
	Args.cc \
	CartInterp.cc \
	Interp.cc \
	Main.cc \
	OutputMdv.cc \
	PpiInterp.cc \
	PolarInterp.cc \
	Radx2Grid.cc \
	ReorderInterp.cc \
	SatInterp.cc \
	SvdData.cc

#
# tdrp macros
#

include $(RAP_MAKE_INC_DIR)/rap_make_tdrp_macros

#
# C++ targets
#

include $(RAP_MAKE_INC_DIR)/rap_make_c++_targets

#
# tdrp targets
#

include $(RAP_MAKE_INC_DIR)/rap_make_tdrp_c++_targets

#
# local targets
#

depend: depend_generic

# DO NOT DELETE THIS LINE -- make depend depends on it.


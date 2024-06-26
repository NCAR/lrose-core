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
# Makefile for Pid2Grid program
#
# Mike Dixon, EOL, NCAR, Boulder, CO, 80307, USA
#
# May 2021
#
###########################################################################

include $(LROSE_CORE_DIR)/build/make_include/lrose_make_macros

TARGET_FILE = Pid2Grid

LOC_INCLUDES = $(NETCDF4_INCS)

LOC_LIBS = \
          -ldsdata -lradar -lFmq -lSpdb -lMdv  -ldsserver -ldidss \
          -lRadx -lNcxx -lrapformats -leuclid -lrapmath \
          -lkd -lphysics -ltoolsa -ldataport -ltdrp \
          $(NETCDF4_LIBS) \
          -lfftw3 -lbz2 -lz -lpthread

LOC_LDFLAGS = $(NETCDF4_LDFLAGS)

LOC_CFLAGS =

HDRS = \
	Params.hh \
	Args.hh \
	CartInterp.hh \
	Interp.hh \
	Orient.hh \
	OutputMdv.hh \
	Pid2Grid.hh \
	PolarCompute.hh \
	PolarThread.hh \
	RhiOrient.hh

CPPC_SRCS = \
	Params.cc \
	Args.cc \
	CartInterp.cc \
	Interp.cc \
	Main.cc \
	Orient.cc \
	OutputMdv.cc \
	PolarCompute.cc \
	PolarThread.cc \
	Pid2Grid.cc \
	RhiOrient.cc

#
# tdrp macros
#

include $(LROSE_CORE_DIR)/build/make_include/lrose_make_tdrp_macros

#
# C++ targets
#

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


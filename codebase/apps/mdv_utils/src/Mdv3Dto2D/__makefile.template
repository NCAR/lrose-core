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
# Makefile for Mdv3Dto2D.
#
# Nancy Rehak, RAP, NCAR, Boulder, CO, 80307, USA
#
# March 2008
#
###########################################################################

include $(LROSE_CORE_DIR)/build/make_include/lrose_make_macros

MODULE_TYPE=progcpp

TARGET_FILE = Mdv3Dto2D

LOC_INCLUDES = $(NETCDF4_INCS)

LOC_LIBS = \
	-ldsdata -lMdv -lSpdb -lFmq \
	-lrapformats -ldsserver -ldidss -leuclid \
	-lrapmath -ltoolsa -ldataport -ltdrp \
	-lRadx -lNcxx -lphysics -lpthread \
	$(NETCDF4_LIBS) -lbz2 -lz

LOC_LDFLAGS = $(NETCDF4_LDFLAGS)

LOC_CFLAGS =
LOC_CPPC_CFLAGS = 

HDRS = \
	Params.hh \
	Args.hh \
	Computer.hh \
	FirstComputer.hh \
	MaxComputer.hh \
	Mdv3Dto2D.hh \
	MeanComputer.hh \
	MinComputer.hh \
	SumComputer.hh

CPPC_SRCS = \
	Params.cc \
	Main.cc \
	Args.cc \
	Computer.cc \
	FirstComputer.cc \
	MaxComputer.cc \
	Mdv3Dto2D.cc \
	MeanComputer.cc \
	MinComputer.cc \
	SumComputer.cc

#
# tdrp macros
#

include $(LROSE_CORE_DIR)/build/make_include/lrose_make_tdrp_macros

#
# standard C++ targetsM
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



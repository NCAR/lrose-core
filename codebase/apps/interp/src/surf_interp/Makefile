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
# Makefile for surf_interp program
#
# Niles Oien, RAP, NCAR, Boulder, CO, 80307, USA
#
# January 2000
#
###########################################################################

include $(RAP_MAKE_INC_DIR)/rap_make_macros

TARGET_FILE = surf_interp

LOC_INCLUDES = $(NETCDF4_INCS)

LOC_LIBS = -lMdv -lRadx -leuclid -lSpdb \
	-ldsserver -ldidss -lrapformats -lrapmath \
	-ltoolsa -ltdrp -ldataport $(NETCDF4_LIBS) \
	-lbz2 -lz -lm

LOC_LDFLAGS = $(NETCDF4_LDFLAGS)

LOC_CFLAGS =


HDRS = \
	$(PARAMS_HH) \
	Args.hh \
	ReadSounding.hh \
	Control.hh \
	Trigger.hh \
	fort_stuff.hh \
	pints.hh \
	nints.hh \
	read_sounding_class.hh \
	calc_li.hh \
	Output.hh \
	GetDataAndWrite.hh \
	Input.hh \
	Terrain.hh \
	Filt2DMedian.hh

CPPC_SRCS = \
	$(PARAMS_CC) \
	GetDataAndWrite.cc \
	Output.cc \
	Main.cc \
	ReadSounding.cc \
	Args.cc \
	Control.cc \
	Trigger.cc \
	read_sounding_class.cc \
	calc_li.cc \
	pints.cc \
	nints.cc \
	fort_stuff.cc \
	Input.cc \
	Terrain.cc \
	Filt2DMedian.cc

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


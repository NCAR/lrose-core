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
# Makefile for Grib2toMdv program
#
# RAP, NCAR, Boulder, CO, 80307, USA
#
# June 2006
#
###########################################################################

include $(RAP_MAKE_INC_DIR)/rap_make_macros

TARGET_FILE = Grib2toMdv
MODULE_TYPE = progcpp

LOC_INCLUDES = $(JASPER_INCLUDES) $(NETCDF4_INCS)

LOC_LIBS = -ldsdata -lMdv -lRadx -lNcxx -ldsserver \
	-ldidss -lrapformats -lgrib2 -leuclid \
	-ltoolsa -lphysics $(JASPER_LIBS) -lpng \
	-ldataport -ltdrp -lrapmath -lNcxx $(NETCDF4_LIBS) \
	-lbz2 -lz -lpthread -lm

LOC_LDFLAGS = $(JASPER_LDFLAGS) $(NETCDF4_LDFLAGS)

LOC_CPPC_CFLAGS =

HDRS = \
	$(PARAMS_HH) \
	Args.hh \
	Grib2Mdv.hh \
	HtInterp.hh \
	OutputFile.hh \
	Grib2toMdv.hh


CPPC_SRCS = \
	$(PARAMS_CC) \
	Args.cc \
	Grib2Mdv.cc \
	HtInterp.cc \
	OutputFile.cc \
	Grib2toMdv.cc \
	main.cc

include $(RAP_MAKE_INC_DIR)/rap_make_tdrp_macros

#
# standard targets
#

include $(RAP_MAKE_INC_DIR)/rap_make_c++_targets

#
# tdrp
#

include $(RAP_MAKE_INC_DIR)/rap_make_tdrp_c++_targets

#
# local targets
#

depend: depend_generic

# DO NOT DELETE THIS LINE -- make depend depends on it.


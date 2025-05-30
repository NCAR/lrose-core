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
# Makefile for MdvExtract1Point
#
# Nancy Rehak, RAP, NCAR, Boulder, CO, 80307, USA
#
# August 2009
#
###########################################################################

include $(LROSE_CORE_DIR)/build/make_include/lrose_make_macros

MODULE_TYPE=progcpp

TARGET_FILE = MdvExtract1Point

LOC_INCLUDES = $(NETCDF4_INCS)

LOC_LIBS = -ldsdata -lMdv -lRadx -lNcxx -lSpdb \
	-ldsserver -ldidss -lrapformats -leuclid \
	-ltoolsa -lpthread -ldataport -ltdrp $(NETCDF4_LIBS) \
	-lbz2 -lz

LOC_LDFLAGS = $(NETCDF4_LDFLAGS)

LOC_CFLAGS = 
LOC_CPPC_CFLAGS = 

HDRS = \
	Params.hh \
	Ascii1Output.hh \
	Args.hh \
	MdvExtract1Point.hh \
	Output.hh

CPPC_SRCS = \
	Params.cc \
	Ascii1Output.cc \
	Args.cc \
	MdvExtract1Point.cc \
	Main.cc \
	Output.cc

#
# standard targets
#

include $(LROSE_CORE_DIR)/build/make_include/lrose_make_targets

#
# local targets
#

depend: depend_generic

#
# TDRP support
#

Params.cc: paramdef.$(TARGET_FILE)
	tdrp_gen -f paramdef.$(TARGET_FILE) -c++ \
		-prog $(TARGET_FILE)

clean_tdrp:
	$(RM) Params.hh Params.cc

# DO NOT DELETE THIS LINE -- make depend depends on it.


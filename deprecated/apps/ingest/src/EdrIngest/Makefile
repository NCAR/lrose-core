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
# Makefile for EdrIngest program
#
# Sue Dettling, RAP, NCAR, Boulder, CO, 80307, USA
#
# November 2004 
#
###########################################################################

include $(LROSE_CORE_DIR)/build/make_include/lrose_make_macros

TARGET_FILE = EdrIngest

LOC_INCLUDES = -I../include -D_LINUX_TYPES_H $(NETCDF4_INCS)

LOC_LIBS = -lSpdb -lrapformats -ldsserver -ldidss \
	-lmel_bufr -ltdrp -ldataport \
	-lInsituTurb -leuclid -ltoolsa -lpthread $(NETCDF4_LIBS) -lbz2 \
	-lz -lm

LOC_LDFLAGS = $(NETCDF4_LDFLAGS)



HDRS = \
	$(PARAMS_HH) \
	Args.hh \
	EdrInput.hh \
	FileInput.hh \
	EdrReport.hh \
	DeltaEdrReport.hh \
	SouthwestEdrReport.hh \
	UnitedEdrReport.hh \
	EdrIngest.hh \
	UalTailMap.hh \
	char_rep.h \
	lscale.h \
	turb.h

CPPC_SRCS = \
	$(PARAMS_CC) \
	Args.cc \
	EdrIngest.cc \
	FileInput.cc \
	EdrReport.cc \
	DeltaEdrReport.cc \
	SouthwestEdrReport.cc \
	UnitedEdrReport.cc \
	UalTailMap.cc \
	char_rep.c \
	Main.cc 

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


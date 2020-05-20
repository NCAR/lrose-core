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
# Makefile for RadxMesoCyclone program
#
# Dave Albo, RAP, NCAR, Boulder, CO, 80307, USA
#
# MAy 2020
#
###########################################################################

include $(LROSE_CORE_DIR)/build/make_include/lrose_make_macros

TARGET_FILE = RadxMesoCyclone

LOC_INCLUDES = $(NETCDF4_INCS)

LOC_LIBS = -lFiltAlgVirtVol -lradar -lphysics \
	-ldsdata -lSpdb -lMdv -lRadx -lNcxx \
	-ldsserver -ldidss -lrapformats -leuclid \
	-lrapmath -ltoolsa -ldataport -ltdrp \
	$(NETCDF4_LIBS) -lbz2 -lz -lpthread \
	-lm

LOC_LDFLAGS = $(NETCDF4_LDFLAGS)

LOC_CFLAGS =

CPPC_SRCS = \
	Params.cc \
	FullTemplateExtent.cc \
	GridExpand.cc \
	Parms.cc \
	Main.cc \
	MesoTemplate.cc \
	NyquistTest.cc \
	RadxMesoCyclone.cc \
	Sweep.cc \
	TemplateLookup.cc \
	TemplateLookup1.cc \
	TemplateLookupMgr.cc \
	Volume.cc

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

###########################################################################
#
# Makefile for RadxMesoCyclone
#
###########################################################################

# include $(LROSE_CORE_DIR)/build/make_include/lrose_make_macros

# MODULE_TYPE=progcpp
# TARGET_FILE=RadxMesoCyclone
# LOC_INCLUDES = -I. $(NETCDF4_INCS)

# LOC_LIBS = -lFiltAlgVirtVol -lradar -lphysics \
# 	-ldsdata -lSpdb -lMdv -lRadx -lNcxx \
# 	-ldsserver -ldidss -lrapformats -leuclid \
# 	-lrapmath -ltoolsa -ldataport -ltdrp \
# 	$(NETCDF4_LIBS) -lbz2 -lz -lpthread \
# 	-lm

# LOC_LDFLAGS = $(NETCDF4_LDFLAGS)

# LOC_INC_DIR = .
# MY_CPPCHECKFLAGS = 
# LOC_CFLAGS = 
# LOC_CPPC_CFLAGS = 

# HDRS = 

# CPPC_SRCS = \
# 	Params.cc \
# 	FullTemplateExtent.cc \
# 	GridExpand.cc \
# 	Parms.cc \
# 	Main.cc \
# 	MesoTemplate.cc \
# 	NyquistTest.cc \
# 	RadxMesoCyclone.cc \
# 	Sweep.cc \
# 	TemplateLookup.cc \
# 	TemplateLookup1.cc \
# 	TemplateLookupMgr.cc \
# 	Volume.cc

# #
# # Standard c++ targets
# #
# include $(RAP_MAKE_INC_DIR)/rap_make_c++_targets

# #
# # local targets
# #
# Params.hh: Params.cc

# Params.cc: paramdef.RadxMesoCyclone
# 	tdrp_gen -f paramdef.RadxMesoCyclone -c++ -class Params


# # DO NOT DELETE THIS LINE -- make depend depends on it.


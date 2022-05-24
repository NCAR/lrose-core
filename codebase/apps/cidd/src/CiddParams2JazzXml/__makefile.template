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
# Makefile for CiddParams2JazzXml.
#
# Nancy Rehak, RAP, NCAR, Boulder, CO, 80307, USA
#
# September 2010
#
###########################################################################

include $(LROSE_CORE_DIR)/build/make_include/lrose_make_macros

MODULE_TYPE=progcpp

TARGET_FILE = CiddParams2JazzXml

LOC_INCLUDES = $(NETCDF4_INCS)

LOC_LIBS = -lMdv -lRadx -lNcxx -ldsserver -ldidss -lrapformats \
	-leuclid -ltoolsa -lpthread -lrapmath -ldataport \
	-ltdrp $(NETCDF4_LIBS) -lbz2 -lz

LOC_LDFLAGS = $(NETCDF4_LDFLAGS)

LOC_CFLAGS =
LOC_CPPC_CFLAGS = 

HDRS = \
	Args.hh \
	CiddParams2JazzXml.hh \
	CiddParamFile.hh \
	GuiConfigParams.hh \
	GridField.hh \
	GridMenu.hh \
	GridsParams.hh \
	JazzXml.hh \
	MainParams.hh \
	MapField.hh \
	MapsParams.hh \
	ParamSection.hh \
	SymprodField.hh \
	SymprodsParams.hh \
	TdrpParamSection.hh \
	TerrainParams.hh \
	Uparams.hh \
	WindField.hh \
	WindsParams.hh \
	Zoom.hh

CPPC_SRCS = \
	Main.cc \
	Args.cc \
	CiddParams2JazzXml.cc \
	CiddParamFile.cc \
	Cgui_P.cc \
	Csyprod_P.cc \
	Cterrain_P.cc \
	GuiConfigParams.cc \
	GridField.cc \
	GridMenu.cc \
	GridsParams.cc \
	JazzXml.cc \
	MainParams.cc \
	MapField.cc \
	MapsParams.cc \
	ParamSection.cc \
	SymprodField.cc \
	SymprodsParams.cc \
	TdrpParamSection.cc \
	TerrainParams.cc \
	Uparams.cc \
	WindField.cc \
	WindsParams.cc \
	Zoom.cc

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

Csyprod_P.cc: paramdef.Cidd_syprods
	tdrp_gen -f paramdef.Cidd_syprods -c++ -class Csyprod_P  -prog CIDD

Cgui_P.cc: paramdef.Cidd_gui
	tdrp_gen -f paramdef.Cidd_gui -c++ -class Cgui_P  -prog CIDD

Cterrain_P.cc: paramdef.Cidd_terrain
	tdrp_gen -f paramdef.Cidd_terrain -c++ -class Cterrain_P  -prog CIDD

depend: cidd_macros.h $(TDRP_C) depend_generic

clean_tdrp:
	$(RM) Csyprod_P.cc Csyprod_P.hh \
	Cgui_P.cc Cgui_P.hh \
	Cterrain_P.cc  Cterrain_P.hh


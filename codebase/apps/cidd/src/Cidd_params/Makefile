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
# Makefile for Cidd_params program
#
###########################################################################

include $(LROSE_MAKE_INC_DIR)/lrose_make_macros

TARGET_FILE = Cidd_params

LOC_INCLUDES = -I../include
LOC_CFLAGS =
LOC_LDFLAGS = -L..
LOC_LIBS = -ltdrp

HDRS = \
	Cidd_params.hh \
	Cdpd_P.hh \
	Cgrid_P.hh \
	Cgui_P.hh \
	Cmap_P.hh \
	Croutes_P.hh \
	Cterrain_P.hh \
	Csyprods_P.hh

# Leave the TDRP sources first 
CPPC_SRCS = \
	Cdpd_P.cc \
	Cgrid_P.cc \
	Cgui_P.cc \
	Cmap_P.cc \
	Csyprod_P.cc  \
	Croutes_P.cc \
	Cterrain_P.cc \
	Args.cc \
	Main.cc \
	Cidd_params.cc


#
# tdrp macros
#

include $(LROSE_MAKE_INC_DIR)/lrose_make_tdrp_macros

#
# C++ targets
include $(LROSE_MAKE_INC_DIR)/lrose_make_c++_targets

#
# local targets
#

Cdpd_P.cc: paramdef.Cidd_dpd
	tdrp_gen -f paramdef.Cidd_dpd -c++ -class Cdpd_P -prog CIDD

Cgrid_P.cc: paramdef.Cidd_grids
	tdrp_gen -f paramdef.Cidd_grids -c++ -class Cgrid_P  -prog CIDD

Cgui_P.cc: paramdef.Cidd_gui
	tdrp_gen -f paramdef.Cidd_gui -c++ -class Cgui_P  -prog CIDD

Cmap_P.cc: paramdef.Cidd_maps
	tdrp_gen -f paramdef.Cidd_maps -c++ -class Cmap_P  -prog CIDD

Csyprod_P.cc: paramdef.Cidd_syprods
	tdrp_gen -f paramdef.Cidd_syprods -c++ -class Csyprod_P  -prog CIDD

Croutes_P.cc: paramdef.Cidd_routes
	tdrp_gen -f paramdef.Cidd_routes -c++ -class Croutes_P  -prog CIDD

Cterrain_P.cc: paramdef.Cidd_terrain
	tdrp_gen -f paramdef.Cidd_terrain -c++ -class Cterrain_P  -prog CIDD

clean_tdrps:
	$(RM) Cdpd_P.hh Cdpd_P.cc Ccscale_P.hh Ccscale_P.cc \
	Cgrid_P.hh Cgrid_P.cc Cgui_P.hh \
	Cmap_P.hh Cmap_P.cc Csyprod_P.hh Csyprod_P.cc 

depend: depend_generic

# DO NOT DELETE THIS LINE -- make depend depends on it.


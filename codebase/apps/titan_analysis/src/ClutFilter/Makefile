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
# Makefile for ClutFilter program
#
# Mike Dixon, RAP, NCAR
# POBox 3000, Boulder, CO, 80307, USA
#
# Feb 2004
#
###########################################################################

include $(LROSE_CORE_DIR)/build/make_include/lrose_make_macros

TARGET_FILE = ClutFilter

LOC_INCLUDES =
LOC_CPPC_CFLAGS =
LOC_LDFLAGS =
#LOC_LIBS = -ltoolsa -lpthread -ldataport -lrapmath -ltdrp -lfftw3
LOC_LIBS = \
	libgmap.a -ltoolsa -lpthread -ldataport -lrapmath \
	-ltdrp -lfftw3 -lbz2 -lz

HDRS = \
	$(PARAMS_HH) \
	Args.hh \
	ClutFilter.hh \
	Fft.hh \
	Filter.hh \
	MeasuredSpec.hh

CPPC_SRCS = \
	$(PARAMS_CC) \
	Args.cc \
	ClutFilter.cc \
	Fft.cc \
	Filter.cc \
	Main.cc \
	MeasuredSpec.cc

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

GmapFilter: FORCE
	$(RM) ClutFilter.o
	$(MAKE) _CC="$(CC)" _CPPC="$(CPPC)" \
		DBUG_OPT_FLAGS="-O2 -DUSE_GMAP" ClutFilter.o
	LD_LIBRARY_PATH=$(LD_LIBRARY_PATH):$(LOC_LD_LIBRARY_PATH):$(SYS_LD_LIBRARY_PATH)
	$(RM) GmapFilter
	$(_CPPC) $(DBUG_OPT_FLAGS) -o GmapFilter -static \
	 	$(OBJS) $(DEBUG_LDFLAGS) $(LDFLAGS) $(LIBS)
	$(RM) ClutFilter.o

FORCE:

# DO NOT DELETE THIS LINE -- make depend depends on it.



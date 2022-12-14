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
# Makefile for RefractParams2Tdrp.
#
# Nancy Rehak, RAP, NCAR, Boulder, CO, 80307, USA
#
# January 2008
#
###########################################################################

include $(LROSE_CORE_DIR)/build/make_include/lrose_make_macros

MODULE_TYPE = progcpp

TARGET_FILE = RefractParams2Tdrp

LOC_INCLUDES = 
LOC_CFLAGS =
LOC_CPPC_CFLAGS = 
LOC_LDFLAGS = 
LOC_LIBS = \
	-ltoolsa -ldataport -ltdrp -lpthread \
	-lbz2 -lz

HDRS = \
	Args.hh \
	RefractCalibParams.hh \
	RefractParams.hh \
	RefractParams2Tdrp.hh

CPPC_SRCS = \
	Main.cc \
	Args.cc \
	RefractCalibParams.cc \
	RefractParams.cc \
	RefractParams2Tdrp.cc

#
# standard targets
#

include $(LROSE_CORE_DIR)/build/make_include/lrose_make_c++_targets

#
# local targets
#

depend: depend_generic

#
# TDRP support
#

RefractParams.cc: paramdef.Refract
	tdrp_gen Refract -f paramdef.Refract -c++ \
		-class RefractParams \
		-prog $(TARGET_FILE) -add_ncar_copyright

RefractCalibParams.cc: paramdef.RefractCalib
	tdrp_gen RefractCalib -f paramdef.RefractCalib -c++ \
		-class RefractCalibParams \
		-prog $(TARGET_FILE) -add_ncar_copyright

clean_tdrp:
	$(RM) Params.hh Params.cc \
		RefractParams.hh RefractParams.cc \
		RefractCalibParams.hh RefractCalibParams.cc

# DO NOT DELETE THIS LINE -- make depend depends on it.



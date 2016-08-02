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
# Makefile for RadxModelQc program
#
# Mike Dixon, RAP, NCAR, Boulder, CO, 80307, USA
#
# Jan 2010
#
###########################################################################

include $(RAP_MAKE_INC_DIR)/rap_make_macros

TARGET_FILE = RadxModelQc

LOC_INC_DIR = .
MY_CPPCHECKFLAGS = -E Params.cc -E Params.hh -AddStyleFAA -exclude-rules="4.16,4.09,4.14,4.12"

LOC_INCLUDES = $(NETCDF4_INCS)

LOC_LIBS = -lradar -lRadx -lSpdb -ldsserver \
	-lrapformats -lrapmath -ldidss -ltoolsa \
	-ltdrp -ldataport -lfftw3 $(NETCDF4_LIBS) \
	-lbz2 -lz -lpthread

LOC_LDFLAGS = $(NETCDF4_LDFLAGS)

LOC_CFLAGS =

HDRS = \
	RadxModelQc.hh

CPPC_SRCS = \
	Params.cc \
	Main.cc \
	RadxModelQc.cc \
	FiltAzGradient.cc \
	FiltClutter2dQual.cc \
	FiltComment.cc \
	FiltCreate.cc \
	FiltMask.cc \
	FiltGriddedMath.cc \
	FiltMath.cc \
	Filt2dGaussianMapping.cc \
	FiltPassThrough.cc \
	FiltQscale.cc \
	FiltSpectrumWidthNorm.cc \
	FiltThresh.cc \
	Filter.cc \
	Info.cc 

X_SRCS = \
	FiltCombine.cc \
	FiltDB.cc \
	FiltDivide.cc \
	FiltDifference.cc \
	Filter.cc \
	FiltFuzzyRemap.cc \
	FiltFuzzy2dRemap.cc \
	FiltMask.cc \
	FiltRemap.cc \
	FiltRestrict.cc \
	FiltSpeckle.cc \

#
# tdrp macros
#

include $(RAP_MAKE_INC_DIR)/rap_make_tdrp_macros

#
# C++ targets
#

include $(RAP_MAKE_INC_DIR)/rap_make_c++_targets

#
# tdrp targets
#

include $(RAP_MAKE_INC_DIR)/rap_make_tdrp_c++_targets

#
# local targets
#

depend: depend_generic

# DO NOT DELETE THIS LINE -- make depend depends on it.


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
# Makefile for mdv2matlab program
#
# Nancy Rehak, RAP, NCAR, Boulder, CO, 80307, USA
#
# May 1998
#
###########################################################################

include $(LROSE_CORE_DIR)/build/make_include/lrose_make_macros

TARGET_FILE = mdv2matlab

#LOC_INCLUDES = -I/tools/matlab-vr14s2/extern/include
#LOC_INCLUDES = -I/tools/matlab6.5/extern/include
#LOC_INCLUDES = -I/tools/matlab/extern/include
LOC_INCLUDES = $(MATLAB_INCLUDES)

LOC_CFLAGS =
LOC_CPPC_CFLAGS =

#LOC_LDFLAGS = -L/tools/matlab6.5/extern/lib/glnx86
#LOC_LDFLAGS = -L/tools/matlab-vr14s2/bin/glnx86
#LOC_LDFLAGS = $(MATLAB_LDFLAGS)
LOC_LDFLAGS = -L/tools/matlab/bin/glnx86


#LOC_LD_LIBRARY_PATH = /tools/matlab6.5/extern/lib/glnx86
#LOC_LD_LIBRARY_PATH = /tools/matlab-vr14s2/bin/glnx86
LOC_LD_LIBRARY_PATH = $(MATLAB_LD_LIBRARY_PATH)

#LD_LIBRARY_PATH     = /tools/matlab-vr14s2/bin/glnx86

# HAD TO OVERRIDE WHAT'S IN /rap/make_macros
MATLAB_LIBS = -lmat -lmx

#LOC_LIBS = \
#	$(TDRP_LIBS) \
#	$(MATLAB_LIBS) \
#	-leng \
#	-lmx \
#	-lut \
#	-lmdv \
#	-lrapformats \
#	-ltoolsa \
#	-ldataport \
#	-lm

# matlab 6.5
#LOC_LIBS = \
#	$(TDRP_LIBS) \
#	$(MATLAB_LIBS) \
#	-lmwcl \
#	-lmwgcl \
#	-leng \
#	-lut \
#	-lmdv \
#	-lrapformats \
#	-ltoolsa \
#	-ldataport \
#	-lm

# matlab R2006b
LOC_LIBS = \
	-ltdrp $(MATLAB_LIBS) -leng -lut \
	-lmdv -lrapformats -ltoolsa -ldataport \
	-lbz2 -lm

#	-lmwgcl \
#	-lmwcl \

MODULE_TYPE=progcpp

HDRS = Mdv2Matlab.h \
	Mdv2MatlabArgs.h \
	Mdv2MatlabParams.h \
	$(TDRP_H)

C_SRCS = \
	$(TDRP_C) 

CPPC_SRCS = \
	Main.cc \
	Mdv2Matlab.cc \
	Mdv2MatlabArgs.cc \
	Mdv2MatlabParams.cc 

#
# tdrp support
#

include $(LROSE_CORE_DIR)/build/make_include/lrose_make_tdrp_macros

#
# standard targets
#

include $(LROSE_CORE_DIR)/build/make_include/lrose_make_targets

#
# local targets
#

depend: $(TDRP_C) depend_generic

include $(LROSE_CORE_DIR)/build/make_include/lrose_make_tdrp_targets

# DO NOT DELETE THIS LINE -- make depend depends on it.


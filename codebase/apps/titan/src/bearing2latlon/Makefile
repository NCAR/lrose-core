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
# Makefile for bearing2latlon program
#
# Mike Dixon, RAP, NCAR, Boulder, CO, 80307, USA
#
# Nov 1995
#
###########################################################################

include $(LROSE_CORE_DIR)/build/make_include/lrose_make_macros

MODULE_TYPE=program
TARGET_FILE = bearing2latlon

LOC_INCLUDES =
LOC_CFLAGS =
LOC_LDFLAGS =
LOC_LIBS = \
	-ltitan -lMdv -lrapformats -ldsserver \
	-ldidss -leuclid -lrapmath -ltoolsa \
	-ldataport -ltdrp -lRadx -lNcxx \
	-lphysics -lpthread -lbz2 -lz \
	-lm $(NETCDF4_LIBS)

HDRS = bearing2latlon.h

SRCS = \
	$(TDRP_C) \
	bearing2latlon.c \
	filter.c \
	parse_args.c \
	tidy_and_exit.c

#
# TDRP - Run-Time Parameters files
#

TDRP_MODULE = $(TARGET_FILE)
TDRP_C = $(TDRP_MODULE)_tdrp.c
TDRP_H = $(TDRP_MODULE)_tdrp.h

TDRP_HDRS = \
	$(TDRP_H) \
	$(TDRP_MODULE)_tdrp_defs.h \
	$(TDRP_MODULE)_tdrp_int.h

TDRP_DEFS = paramdef.$(TDRP_MODULE)
TDRP_LIBS = -ltdrp

#
# standard targets
#

include $(LROSE_CORE_DIR)/build/make_include/lrose_make_c_targets

#
# local targets
#

depend: $(TDRP_C) depend_generic

$(TDRP_H): $(TDRP_C)

$(TDRP_C): $(TDRP_DEFS)
	tdrp_gen $(TDRP_MODULE) -f $(TDRP_DEFS)

clean_tdrp::
	$(RM) $(TDRP_HDRS) $(TDRP_C)

# DO NOT DELETE THIS LINE -- make depend depends on it.



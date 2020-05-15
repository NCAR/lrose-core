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
# Makefile for trec program
#
# Laura Stoltz, RAP, NCAR, Boulder, CO, 80307, USA
#
# February 1996
#
###########################################################################

include $(LROSE_CORE_DIR)/build/make_include/lrose_make_macros

MODULE_TYPE=program
TARGET_FILE = trec

LOC_INCLUDES = 
LOC_CFLAGS = 
#LOC_CFLAGS = -O3
LOC_LDFLAGS = 
LOC_LIBS = \
	-ltitan -ltdrp -ltoolsa -lpthread -ldataport \
	-lbz2 -lz -lm

HDRS = \
	cedric.h \
	trec.h

SRCS = \
	$(TDRP_C) \
	adjust_time.c \
	azindx.c \
	deci.c \
	despike.c \
	fill3d.c \
	init_ls.c \
	lstsq.c \
	parse_args.c \
	print_beam.c \
	process_beams.c \
	radius.c \
	read_params.c \
	read_udp_beam.c \
	tidy_and_exit.c \
	tkcomp.c \
	trec.c \
	vcterp.c \
	wair.c \
	write_ced_file.c

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


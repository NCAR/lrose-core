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
# Makefile for compression module
#
# Mike Dixon, RAP, NCAR, Boulder, Co, USA, 80307
#
# July 1999
#
###########################################################################

include $(RAP_MAKE_INC_DIR)/rap_make_macros

LOC_INCLUDES = -I../include
#LOC_CFLAGS = -ansi
LOC_CFLAGS =

TARGET_FILE = ../libtoolsa.a

#
# file lists
#

HDRS = \
	../include/toolsa/lzo_compress.h

SRCS = \
	bzip_compress.c \
	gzip_compress.c \
	lzo_compress.c \
	minilzo.c \
	rle_compress.c \
	ta_compress.c \
	zlib_compress.c

#
# general targets
#

include $(RAP_MAKE_INC_DIR)/rap_make_lib_module_targets

#
# local targets
#

test: test_p

test_p:
	$(MAKE) DBUG_OPT_FLAGS="$(DEBUG_FLAG)" test_bzip
	$(MAKE) DBUG_OPT_FLAGS="$(DEBUG_FLAG)" test_gzip
	$(MAKE) DBUG_OPT_FLAGS="$(DEBUG_FLAG)" test_lzo
	$(MAKE) DBUG_OPT_FLAGS="$(DEBUG_FLAG)" test_rle
	$(MAKE) DBUG_OPT_FLAGS="$(DEBUG_FLAG)" test_zlib

test_bzip: test_bzip.o
	$(CC) $(DBUG_OPT_FLAGS) test_bzip.o \
	$(LDFLAGS) -o test_bzip ../libtoolsa.a -ldataport -lm

test_gzip: test_gzip.o
	$(CC) $(DBUG_OPT_FLAGS) test_gzip.o \
	$(LDFLAGS) -o test_gzip ../libtoolsa.a -ldataport -lm

test_lzo: test_lzo.o
	$(CC) $(DBUG_OPT_FLAGS) test_lzo.o \
	$(LDFLAGS) -o test_lzo ../libtoolsa.a -ldataport -lm

test_rle: test_rle.o
	$(CC) $(DBUG_OPT_FLAGS) test_rle.o \
	$(LDFLAGS) -o test_rle ../libtoolsa.a -ldataport -lm

test_zlib: test_zlib.o
	$(CC) $(DBUG_OPT_FLAGS) test_zlib.o \
	$(LDFLAGS) -o test_zlib ../libtoolsa.a -ldataport -lm

clean_test:
	$(RM) test_bzip test_gzip test_lzo test_rle test_zlib

depend: depend_generic

# DO NOT DELETE THIS LINE -- make depend depends on it.

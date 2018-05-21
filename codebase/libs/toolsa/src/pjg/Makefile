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
# Makefile for projective geometry module
#
# Mike Dixon, RAP, NCAR, Boulder, Co, USA, 80307
#
# Nov 1994
#
###########################################################################

include $(RAP_MAKE_INC_DIR)/rap_make_macros

LOC_INCLUDES = -I../include
LOC_CFLAGS =

TARGET_FILE = ../libtoolsa.a

#
# file lists
#

HDRS = \
	../include/toolsa/pjg.h \
	pjg_int.h

SRCS = \
	pjg.c \
	pjg_art.c \
	pjg_cylind.c \
	pjg_flat.c \
	pjg_grid.c \
	pjg_lc2.c \
	pjg_ll.c \
	pjg_mdv.c \
	pjg_ps.c \
	pjg_pse.c \
	pjg_tm.c \
	pjg_util.c \
	sincos.c

CPPC_SRCS = \
	TaPjg.cc

# testing

TEST_PJG = test_pjg
TEST_PJG_OBJS = TEST_toolsa_pjg.o test_rand.o

#
# general targets
#

include $(RAP_MAKE_INC_DIR)/rap_make_lib_module_targets

#
# testing
#

test: test_p

test_p:
	$(MAKE) DBUG_OPT_FLAGS="$(DEBUG_FLAG)" $(TEST_PJG)

$(TEST_PJG): $(TEST_PJG_OBJS)
	$(CC) $(DBUG_OPT_FLAGS) $(TEST_PJG_OBJS) \
	$(LDFLAGS) -o $(TEST_PJG) -ltoolsa -lm

clean_test:
	$(RM) $(TEST_PJG)

#
# local targets
#

depend: depend_generic

# DO NOT DELETE THIS LINE -- make depend depends on it.

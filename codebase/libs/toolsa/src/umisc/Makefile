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
# Makefile for miscellaneous utility library
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

HDRS = \
	../include/toolsa/umisc.h

SRCS = \
	fsleep.c \
	safe_system.c \
	safe_system_f.c \
	ucopyright.c \
	udaemonize.c \
	ufread.c \
	ufree_parsed_path.c \
	ufwrite.c \
	uparams_etc.c \
	uparse_path.c \
	ushmem.c \
	ustring.c \
	usystem_call.c \
	uusleep.c

CPPC_SRCS = \
	gridLineConnect.cc \
	ugetenv.cc \
	Server.cc \
	ArchiveDates.cc 
#
# general targets
#

include $(RAP_MAKE_INC_DIR)/rap_make_lib_module_targets

#
# testing
#

TEST_RECV = test_umsg_recv
TEST_RECV_OBJS = test_umsg_recv.o

TEST_SEND = test_umsg_send
TEST_SEND_OBJS = test_umsg_send.o

test: test_r test_s

test_r:
	$(MAKE) DBUG_OPT_FLAGS="$(DEBUG_FLAG)" $(TEST_RECV)

test_s:
	$(MAKE) DBUG_OPT_FLAGS="$(DEBUG_FLAG)" $(TEST_SEND)

$(TEST_RECV): $(TEST_RECV_OBJS)
	$(CC) $(DBUG_OPT_FLAGS) $(TEST_RECV_OBJS) \
	$(LDFLAGS) -o $(TEST_RECV) -ltoolsa -lm

$(TEST_SEND): $(TEST_SEND_OBJS)
	$(CC) $(DBUG_OPT_FLAGS) $(TEST_SEND_OBJS) \
	$(LDFLAGS) -o $(TEST_SEND) -ltoolsa -lm

#
# local targets
#

depend: depend_generic

# DO NOT DELETE THIS LINE -- make depend depends on it.

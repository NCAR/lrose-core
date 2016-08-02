# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
# ** Copyright UCAR (c) 1992 - 2010 
# ** University Corporation for Atmospheric Research(UCAR) 
# ** National Center for Atmospheric Research(NCAR) 
# ** Research Applications Laboratory(RAL) 
# ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA 
# ** 2010/10/7 23:12:57 
# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
###########################################################################
#
# Makefile for DsInputPath
#
# Paddy McCarthy, RAP, NCAR, Boulder, Co, USA, 80307
#
# December 1998
#
###########################################################################

include $(RAP_MAKE_INC_DIR)/rap_make_macros

TARGET_FILE = ../libdsserver.a

# Standard Config
LOC_INCLUDES = -I../include
# SBC Config
#LOC_INCLUDES = -I../include -DDISABLE_PMU
#
LOC_CFLAGS = 

HDRS = \
	../include/dsserver/DsProcessServer.hh \
	../include/dsserver/DsServer.hh \
	../include/dsserver/DsServerMsg.hh \
	../include/dsserver/DsThreadedServer.hh \
	../include/dsserver/ProcessServer.hh

CPPC_SRCS = \
	DsProcessServer.cc \
	DsServer.cc \
	DsServerMsg.cc \
	DsThreadedServer.cc \
	ProcessServer.cc

#
# general targets
#

include $(RAP_MAKE_INC_DIR)/rap_make_lib_module_targets

#
# local targets
#

depend: depend_generic

# DO NOT DELETE THIS LINE -- make depend depends on it.

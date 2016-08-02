# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
# ** Copyright UCAR (c) 1992 - 2010 
# ** University Corporation for Atmospheric Research(UCAR) 
# ** National Center for Atmospheric Research(NCAR) 
# ** Research Applications Laboratory(RAL) 
# ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA 
# ** 2010/10/7 23:12:56 
# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
###########################################################################
#
# Makefile for DsFmq class
#
# Terri Betancourt, RAP, NCAR, Boulder, Co, USA, 80307
#
# February 1999
#
###########################################################################

include $(RAP_MAKE_INC_DIR)/rap_make_macros

LOC_INCLUDES = -I../include
LOC_CFLAGS =

TARGET_FILE = ../libFmq.a

#
# file lists
#

HDRS = \
	$(RAP_INC_DIR)/Fmq/DsFmq.hh \
	$(RAP_INC_DIR)/Fmq/FmqMgr.hh \
	$(RAP_INC_DIR)/Fmq/FmqMgrLocal.hh \
	$(RAP_INC_DIR)/Fmq/FmqMgrSocket.hh \
	$(RAP_INC_DIR)/Fmq/FmqMgrClient.hh \
	$(RAP_INC_DIR)/Fmq/FmqMgrServer.hh

CPPC_SRCS = \
	DsFmq.cc \
	DsFmqMsg.cc

#
# general targets
#

include $(RAP_MAKE_INC_DIR)/rap_make_lib_module_targets

#
# local targets
#

depend: depend_generic

# DO NOT DELETE THIS LINE -- make depend depends on it.

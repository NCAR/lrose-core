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

include $(LROSE_CORE_DIR)/build/make_include/lrose_make_macros

LOC_INCLUDES = -I../include
LOC_CFLAGS =

TARGET_FILE = ../libFmq.a

#
# file lists
#

HDRS = \
	$(LROSE_INSTALL_DIR)/include/Fmq/DsFmq.hh \
	$(LROSE_INSTALL_DIR)/include/Fmq/FmqMgr.hh \
	$(LROSE_INSTALL_DIR)/include/Fmq/FmqMgrLocal.hh \
	$(LROSE_INSTALL_DIR)/include/Fmq/FmqMgrSocket.hh \
	$(LROSE_INSTALL_DIR)/include/Fmq/FmqMgrClient.hh \
	$(LROSE_INSTALL_DIR)/include/Fmq/FmqMgrServer.hh

CPPC_SRCS = \
	DsFmq.cc \
	DsFmqMsg.cc

#
# general targets
#

include $(LROSE_CORE_DIR)/build/make_include/lrose_make_lib_module_targets

#
# local targets
#

depend: depend_generic

# DO NOT DELETE THIS LINE -- make depend depends on it.

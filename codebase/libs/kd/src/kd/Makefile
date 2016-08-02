# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
# ** Copyright UCAR (c) 1992 - 2001 
# ** University Corporation for Atmospheric Research(UCAR) 
# ** National Center for Atmospheric Research(NCAR) 
# ** Research Applications Program(RAP) 
# ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA 
# ** 2001/12/18 19:51:15 
# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
###########################################################################
#
# Makefile for kd module
#
# Gerry Wiener, RAP, NCAR, Boulder, Co, USA, 80307
#
# Adapted from ranger code:
#
# http://www.cs.sunysb.edu/~algorith/implement/ranger/implement.shtml
#
#            The Algorithm Design Manual 
#         by Steven S. Skiena, Steve Skiena 
#
###########################################################################


include $(RAP_MAKE_INC_DIR)/rap_make_macros

LOC_INCLUDES = -I../include
LOC_CPPC_CFLAGS =  -g -O

TARGET_FILE = ../libkd.a
MODULE_TYPE = library

HDRS = \
	../include/kd/kd.hh \
	../include/kd/fileoper.hh \
	../include/kd/naive.hh \
	../include/kd/metric.hh \
	../include/kd/pqueue.hh \
	../include/kd/kd_interp.hh \
	../include/kd/kd_query.hh \
	../include/kd/tokenize.hh

CPPC_SRCS = \
	fileoper.cc \
	kd.cc \
	metric.cc \
	naive.cc \
	pqueue.cc \
	kd_interp.cc \
	kd_query.cc \
	tokenize.cc


#
# general targets
#

include $(RAP_MAKE_INC_DIR)/rap_make_lib_module_targets


#
# local targets
#

test_kd: test_kd.o
	$(CPPC) $(LOC_CPPC_CFLAGS) test_kd.o ../libkd.a -o test_kd

test_kd_interp: test_kd_interp.o
	$(CPPC) $(LOC_CPPC_CFLAGS) test_kd_interp.o ../libkd.a -o test_kd_interp

test_kd_query: test_kd_query.o
	$(CPPC) $(LOC_CPPC_CFLAGS) test_kd_query.o ../libkd.a -o test_kd_query

time_test_kd: time_test_kd.o
	$(CPPC) $(LOC_CPPC_CFLAGS) time_test_kd.o ../libkd.a -o time_test_kd

find_nearest: find_nearest.o
	$(CPPC) $(LOC_CPPC_CFLAGS) find_nearest.o ../libkd.a -o find_nearest

depend: depend_generic

# DO NOT DELETE THIS LINE -- make depend depends on it.

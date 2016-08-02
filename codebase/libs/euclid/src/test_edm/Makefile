# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
# ** Copyright UCAR (c) 1992 - 2010 
# ** University Corporation for Atmospheric Research(UCAR) 
# ** National Center for Atmospheric Research(NCAR) 
# ** Research Applications Laboratory(RAL) 
# ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA 
# ** 2010/10/7 23:12:39 
# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
###########################################################################
#
# Makefile for test_edm program
#
# Mike Dixon, RAP, NCAR, Boulder, CO, 80307, USA
#
# Nov 1994
#
###########################################################################

include $(RAP_MAKE_INC_DIR)/rap_make_macros

TARGET_FILE = test_edm

LOC_INCLUDES = -I../include
LOC_CFLAGS =
LOC_LDFLAGS = -L..
LOC_LIBS = -leuclid -lm

HDRS =

SRCS = \
	test_edm.c

#
# standard targets
#


include $(RAP_MAKE_INC_DIR)/rap_make_lib_module_targets

#
# local targets
#

test_edm: test_edm.o
	$(CC) test_edm.o $(LOC_LDFLAGS) $(LOC_LIBS) -o test_edm

depend: depend_generic

# DO NOT DELETE THIS LINE -- make depend depends on it.

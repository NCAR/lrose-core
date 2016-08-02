# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
# ** Copyright UCAR (c) 1992 - 2010 
# ** University Corporation for Atmospheric Research(UCAR) 
# ** National Center for Atmospheric Research(NCAR) 
# ** Research Applications Laboratory(RAL) 
# ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA 
# ** 2010/10/7 23:12:29 
# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
###########################################################################
#
# Makefile for FORTRAN GRIB Library
#
# Greg Thompson, RAP, NCAR, Boulder, Co, USA, 80307
#
# July 1999
#
###########################################################################

include $(RAP_MAKE_INC_DIR)/rap_make_macros

TARGET_FILE = ../libgrib.a
MODULE_TYPE=library

LOC_INCLUDES =
LOC_CFLAGS =

# The above is an environment variable useful for programs
# where Fortran calls C-routines.  Its value is FORTRAN_C=F_UNDERSCORE
# On Linux boxes, if a C-routine has an underscore in its name
# then the compiler renames the C-routine with two underscores
# appended.  This is annoying as all other machines use a single
# underscore.  For this reason, it is much more desireable not
# to name the C routines with an underscore.
# Alternatively, we could use the compiler option -fno-double-underscore
# to prevent the two underscore appendage problem (a single one is used
# instead).

# If running this code on a 64-bit machine, then use the macro
# definition -DBIT64 for the bit_utils code to work properly.
# This macro is already defined for LINUX_ALPHA make include file.

HDRS =

C_SRCS = \
	cmem_utils.c \
	io_utils.c

F_CPP_SRCS = \
	bit_utils.F \
	w3fi58.F

F_SRCS = \
	w3fi63.f \
	gribsys.f \
	gributil.f \
	w3fi68.f \
	w3fi71.f \
	w3fi72.f \
	w3fi73.f \
	w3fi74.f \
	w3fi76.f \
	w3fi82.f \
	xstore.f \
	xmovex.f

#
# general targets
#

include $(RAP_MAKE_INC_DIR)/rap_make_lib_module_targets

#
# local targets
#

depend: depend_generic

# DO NOT DELETE THIS LINE -- make depend depends on it.

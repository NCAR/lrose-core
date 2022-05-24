# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
# ** Copyright UCAR (c) 1992 - 2010 
# ** University Corporation for Atmospheric Research(UCAR) 
# ** National Center for Atmospheric Research(NCAR) 
# ** Research Applications Laboratory(RAL) 
# ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA 
# ** 2010/10/7 23:12:54 
# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
###########################################################################
#
# Makefile for math utility library
#
# Mike Dixon, RAP, NCAR, Boulder, Co, USA, 80307
#
# March 1993
#
###########################################################################

include $(LROSE_CORE_DIR)/build/make_include/lrose_make_macros

LOC_INCLUDES = -I../include
LOC_CFLAGS =
FFLAGS = -ffree-form

TARGET_FILE = ../librapmath.a

HDRS = ../include/rapmath/umath.h \
       ../include/rapmath/usort.h \
       ../include/rapmath/usvd.h

CPPC_SRCS = \
	ForsytheFit.cc \
	PlaneFit.cc \
	PolyFit.cc \
	usort_stl.cc \
	uCurveFit.cc \
	uNewtRaph.cc \
	uPct.cc

SRCS = \
	dir_speed_2_uv.c \
	uLatLon.c \
	uintersect.c \
	uline_through_ellipse.c \
	ulong_scale.c \
	umax_wt_bip.c \
	upoint_in_ellipse.c \
	upoint_in_polygon.c \
	usign.c \
	usort.c \
	ustat.c \
	usvd.c \
	uv_2_dir_speed.c

# F_SRCS = ForsytheFortran.f

#
# general targets
#

include $(LROSE_CORE_DIR)/build/make_include/lrose_make_lib_module_targets

#
# testing
#

test: utest_p

utest_p:
	$(MAKE) DBUG_OPT_FLAGS="$(DEBUG_FLAG)" utest testPlaneFit

utest: uMathTest.o TestPlaneFit.o
	$(CPPC) $(DBUG_OPT_FLAGS) -I../include/rapmath uMathTest.o \
	$(LDFLAGS) -o uMathTest -lrapmath -ltoolsa
	$(CPPC) $(DBUG_OPT_FLAGS) -I../include/rapmath TestPlaneFit.o \
	$(LDFLAGS) -o testPlaneFit -lrapmath -ltoolsa

clean_test:
	$(RM) uMathTest uMathTest.o testPlaneFit TestPlaneFit.o

#
# local targets
#

depend: depend_generic

# DO NOT DELETE THIS LINE -- make depend depends on it.

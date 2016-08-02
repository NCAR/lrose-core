# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
# ** Copyright UCAR (c) 1992 - 2010 
# ** University Corporation for Atmospheric Research(UCAR) 
# ** National Center for Atmospheric Research(NCAR) 
# ** Research Applications Laboratory(RAL) 
# ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA 
# ** 2010/10/7 23:12:40 
# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
###########################################################################
#
# Makefile for the Pjg (ProJective Geometry) module of the euclid library
#
# Nancy Rehak, RAP, NCAR, Boulder, Co, USA, 80307
#
# April 2001
#
###########################################################################

include $(RAP_MAKE_INC_DIR)/rap_make_macros

LOC_INCLUDES = -I../include
LOC_CPPC_CFLAGS =

TARGET_FILE = ../libeuclid.a
MODULE_TYPE = library

#
# file lists
#

HDRS = \
	../include/euclid/Pjg.hh \
	../include/euclid/PjgTypes.hh \
	../include/euclid/PjgCalc.hh \
	../include/euclid/PjgFlatCalc.hh \
	../include/euclid/PjgLatlonCalc.hh \
	../include/euclid/PjgLc1Calc.hh \
	../include/euclid/PjgLc2Calc.hh \
	../include/euclid/PjgPolarRadarCalc.hh \
	../include/euclid/PjgGribCalc.hh \
	../include/euclid/PjgPolarStereoCalc.hh \
	../include/euclid/PjgObliqueStereoCalc.hh \
	../include/euclid/PjgMercatorCalc.hh 

CPPC_SRCS = \
	Pjg.cc \
	PjgAlbersMath.cc \
	PjgAzimEquidistMath.cc \
	PjgCalc.cc \
	PjgFlatCalc.cc \
	PjgGrib.cc \
	PjgLambertAzimMath.cc \
	PjgLambertConfMath.cc \
	PjgLatlonCalc.cc \
	PjgLatlonMath.cc \
	PjgLc1Calc.cc \
	PjgLc2Calc.cc \
	PjgMath.cc \
	PjgMercatorCalc.cc \
	PjgMercatorMath.cc \
	PjgObliqueStereoCalc.cc \
	PjgObliqueStereoMath.cc \
	PjgPolarRadarCalc.cc \
	PjgPolarRadarMath.cc \
	PjgPolarStereoCalc.cc \
	PjgPolarStereoMath.cc \
	PjgTransMercatorMath.cc \
	PjgVertPerspMath.cc

#
# general targets
#

include $(RAP_MAKE_INC_DIR)/rap_make_lib_module_targets

#
# testing
#

test: test_math_p

test_math_p:
	$(MAKE) DBUG_OPT_FLAGS="$(DEBUG_FLAG)" test_math

test_math: TEST_pjg_math.o
	$(CPPC) $(DBUG_OPT_FLAGS) TEST_pjg_math.o \
	$(LDFLAGS) -o test_math -leuclid -lm

clean_test:
	$(RM) test_math TEST_pjg_math.o
	$(RM) *errlog

#
# local targets
#

depend: depend_generic

# DO NOT DELETE THIS LINE -- make depend depends on it.

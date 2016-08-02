# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
# ** Copyright UCAR (c) 1992 - 2010 
# ** University Corporation for Atmospheric Research(UCAR) 
# ** National Center for Atmospheric Research(NCAR) 
# ** Research Applications Laboratory(RAL) 
# ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA 
# ** 2010/10/7 23:12:55 
# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
###########################################################################
#
# Makefile for functions module of rapmath library
#
# Nancy Rehak, RAP, NCAR, Boulder, Co, USA, 80307
#
# August 1999
#
###########################################################################

include $(RAP_MAKE_INC_DIR)/rap_make_macros

LOC_INCLUDES = -I../include
LOC_CFLAGS =

TARGET_FILE = ../librapmath.a
MODULE_TYPE=library

#
# file lists
#

HDRS = \
	$(RAP_INC_DIR)/rapmath/PtFunction.hh \
	$(RAP_INC_DIR)/rapmath/LagrangePtFunc.hh \
	$(RAP_INC_DIR)/rapmath/LinearPtFunc.hh \
	$(RAP_INC_DIR)/rapmath/NewtonPtFunc.hh \
	$(RAP_INC_DIR)/rapmath/FuzzyFunction.hh \
	$(RAP_INC_DIR)/rapmath/FuzzyF.hh \
	$(RAP_INC_DIR)/rapmath/Fuzzy2d.hh \
	$(RAP_INC_DIR)/rapmath/ParamsFuzzyFunction.hh \
	$(RAP_INC_DIR)/rapmath/PtFuzzyFunc.hh \
	$(RAP_INC_DIR)/rapmath/ScaleFuzzyFunc.hh \
	$(RAP_INC_DIR)/rapmath/StepPtFunc.hh

CPPC_SRCS = \
	PtFunction.cc \
	LagrangePtFunc.cc \
	LinearPtFunc.cc \
	NewtonPtFunc.cc \
	FuzzyF.cc \
	Fuzzy2d.cc \
	FuzzyFunction.cc \
	ParamsFuzzyFunction.cc \
	PtFuzzyFunc.cc \
	ScaleFuzzyFunc.cc \
	StepPtFunc.cc \
	SFuzzyF.cc \
	TrapFuzzyF.cc

#
# general targets
#

include $(RAP_MAKE_INC_DIR)/rap_make_lib_module_targets

#
# local targets
#
ParamsFuzzyFunction.hh: ParamsFuzzyFunction.cc
	$(CP) ParamsFuzzyFunction.hh ../include/rapmath
ParamsFuzzyFunction.cc: paramdef.FuzzyFunction
	tdrp_gen -f paramdef.FuzzyFunction -c++ -class ParamsFuzzyFunction
	$(CP) ParamsFuzzyFunction.hh ../include/rapmath

depend: depend_generic

# DO NOT DELETE THIS LINE -- make depend depends on it.

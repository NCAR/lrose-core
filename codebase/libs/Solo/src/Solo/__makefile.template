# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
# ** Copyright UCAR (c) 1992 - 2010 
# ** University Corporation for Atmospheric Research(UCAR) 
# ** National Center for Atmospheric Research(NCAR) 
# ** Research Applications Laboratory(RAL) 
# ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA 
# ** 2010/10/7 23:12:35 
# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
###########################################################################
#
# Mike Dixon, RAP, NCAR, Boulder, Co, USA, 80307
#
# Jan 2010
#
###########################################################################

include $(LROSE_CORE_DIR)/build/make_include/lrose_make_macros

TARGET_FILE = ../libSolo.a

LOC_INCLUDES = -I../include 

# $(NETCDF4_INCS)

LOC_CFLAGS = 

HDRS = \
        ../include/Solo/GeneralDefinitions.hh \
        ../include/Solo/SoloFunctionsApi.hh \
        ../include/Solo/SoloFunctions.hh \
        ../include/Solo/PointInSpace.hh \
        ../include/Solo/OneBoundary.hh \
        ../include/Solo/BoundaryStuff.hh \
        ../include/Solo/BoundaryPointManagement.hh \
        ../include/Solo/BoundaryPointMap.hh \
        ../include/Solo/BoundaryHeader.hh
	
CPPC_SRCS = \
        PointInSpace.cc \
        OneBoundary.cc \
        BoundaryPointManagement.cc \
        BoundaryPointMap.cc \
	Boundary.cc \
        SoloFunctionsApi.cc \
	Despeckle.cc \
	FlagOps.cc \
	FlaggedAssign.cc \
        RemoveAcMotion.cc \
	BBUnfolding.cc \
	ThresholdField.cc \
	ForcedUnfolding.cc \
	MergeFields.cc \
	NavigationCorrection.cc \
	RemoveRing.cc \
	RainRate.cc \
	Abs.cc \
	Exp.cc \
	RadialShear.cc \
	RemoveSurface.cc


	# AddSubMultFields.cc \
	# FixVortexVelocities.cc \
	# AddMultFields.cc \
	# AddValue.cc \
	# ZLinearize.cc \
	# CopyOps.cc \
#
# general targets
#

include $(LROSE_CORE_DIR)/build/make_include/lrose_make_lib_module_targets

#
# local targets
#

depend: depend_generic

shared_lib:
	g++ -Wall -shared Despeckle.cc -lc -o LibSolo.so

# DO NOT DELETE THIS LINE -- make depend depends on it.

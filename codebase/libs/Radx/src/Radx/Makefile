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

include $(RAP_MAKE_INC_DIR)/rap_make_macros

TARGET_FILE = ../libRadx.a

LOC_INCLUDES = -I../include $(NETCDF4_INCS)
LOC_CFLAGS = 

HDRS = \
	../include/Radx/Radx.hh \
	../include/Radx/RadxAzElev.hh \
	../include/Radx/RadxBuf.hh \
	../include/Radx/RadxComplex.hh \
	../include/Radx/RadxEvent.hh \
	../include/Radx/RadxField.hh \
	../include/Radx/RadxFile.hh \
	../include/Radx/RadxFuzzyF.hh \
	../include/Radx/RadxFuzzy2d.hh \
	../include/Radx/RadxGeoref.hh \
	../include/Radx/RadxMsg.hh \
	../include/Radx/RadxPacking.hh \
	../include/Radx/RadxPath.hh \
	../include/Radx/RadxPlatform.hh \
	../include/Radx/RadxRangeGeom.hh \
	../include/Radx/RadxRay.hh \
	../include/Radx/RadxRcalib.hh \
	../include/Radx/RadxReadDir.hh \
	../include/Radx/RadxRemap.hh \
	../include/Radx/RadxStr.hh \
	../include/Radx/RadxSweep.hh \
	../include/Radx/RadxTime.hh \
	../include/Radx/RadxTimeList.hh \
	../include/Radx/RadxVol.hh \
	../include/Radx/RadxXml.hh \
	../include/Radx/RayxData.hh \
	../include/Radx/RayxMapping.hh

CPPC_SRCS = \
	ByteOrder.cc \
	PseudoRhi.cc \
	Radx.cc \
	RadxAngleHist.cc \
	RadxAzElev.cc \
	RadxBuf.cc \
	RadxComplex.cc \
	RadxCfactors.cc \
	RadxEvent.cc \
	RadxField.cc \
	RadxFile.cc \
	RadxFuzzyF.cc \
	RadxFuzzy2d.cc \
	RadxGeoref.cc \
	RadxMsg.cc \
	RadxPacking.cc \
	RadxPath.cc \
	RadxPlatform.cc \
	RadxRangeGeom.cc \
	RadxRay.cc \
	RadxRcalib.cc \
	RadxReadDir.cc \
	RadxRemap.cc \
	RadxSweep.cc \
	RadxStr.cc \
	RadxTime.cc \
	RadxTimeList.cc \
	RadxVol.cc \
	RadxXml.cc \
	RayxData.cc \
	RayxMapping.cc

#
# general targets
#

include $(RAP_MAKE_INC_DIR)/rap_make_lib_module_targets

#
# local targets
#

depend: depend_generic

# DO NOT DELETE THIS LINE -- make depend depends on it.

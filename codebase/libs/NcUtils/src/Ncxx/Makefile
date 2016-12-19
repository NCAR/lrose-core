# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
# ** Copyright UCAR (c) 1992 - 2010 
# ** University Corporation for Atmospheric Research(UCAR) 
# ** National Center for Atmospheric Research(NCAR) 
# ** Research Applications Laboratory(EOL) 
# ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA 
# ** 2010/10/7 23:12:35 
# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
###########################################################################
#
# Makefile for Ncxx objects
#
# Mike Dixon, EOL, NCAR, Boulder, Co, USA, 80307
#
# Dec 2016
#
###########################################################################

include $(RAP_MAKE_INC_DIR)/rap_make_macros

TARGET_FILE = ../libNcxxUtils.a

LOC_INCLUDES = -I../include $(NETCDF4_INCS)
LOC_CFLAGS = 

HDRS = \
	../include/NcUtils/Ncxx.hh \
	../include/NcUtils/NcxxAtt.hh \
	../include/NcUtils/NcxxByte.hh \
	../include/NcUtils/NcxxChar.hh \
	../include/NcUtils/NcxxCheck.hh \
	../include/NcUtils/NcxxCompoundType.hh \
	../include/NcUtils/NcxxDim.hh \
	../include/NcUtils/NcxxDouble.hh \
	../include/NcUtils/NcxxEnumType.hh \
	../include/NcUtils/NcxxErrStr.hh \
	../include/NcUtils/NcxxException.hh \
	../include/NcUtils/NcxxFile.hh \
	../include/NcUtils/NcxxFloat.hh \
	../include/NcUtils/NcxxGroup.hh \
	../include/NcUtils/NcxxGroupAtt.hh \
	../include/NcUtils/NcxxInt.hh \
	../include/NcUtils/NcxxInt64.hh \
	../include/NcUtils/NcxxOpaqueType.hh \
	../include/NcUtils/NcxxShort.hh \
	../include/NcUtils/NcxxString.hh \
	../include/NcUtils/NcxxType.hh \
	../include/NcUtils/NcxxUbyte.hh \
	../include/NcUtils/NcxxUint.hh \
	../include/NcUtils/NcxxUint64.hh \
	../include/NcUtils/NcxxUshort.hh \
	../include/NcUtils/NcxxVar.hh \
	../include/NcUtils/NcxxVarAtt.hh \
	../include/NcUtils/NcxxVlenType.hh

CPPC_SRCS = \
	Ncxx.cc \
	NcxxAtt.cc \
	NcxxByte.cc \
	NcxxChar.cc \
	NcxxCheck.cc \
	NcxxCompoundType.cc \
	NcxxDim.cc \
	NcxxDouble.cc \
	NcxxEnumType.cc \
	NcxxErrStr.cc \
	NcxxException.cc \
	NcxxFile.cc \
	NcxxFloat.cc \
	NcxxGroup.cc \
	NcxxGroupAtt.cc \
	NcxxInt.cc \
	NcxxInt64.cc \
	NcxxOpaqueType.cc \
	NcxxShort.cc \
	NcxxString.cc \
	NcxxType.cc \
	NcxxUbyte.cc \
	NcxxUint.cc \
	NcxxUint64.cc \
	NcxxUshort.cc \
	NcxxVar.cc \
	NcxxVarAtt.cc \
	NcxxVlenType.cc \

#
# general targets
#

include $(RAP_MAKE_INC_DIR)/rap_make_lib_module_targets

#
# local targets
#

depend: depend_generic

# DO NOT DELETE THIS LINE -- make depend depends on it.

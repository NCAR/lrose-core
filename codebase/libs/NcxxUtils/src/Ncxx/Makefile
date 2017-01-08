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
	../include/NcxxUtils/Ncxx.hh \
	../include/NcxxUtils/NcxxAtt.hh \
	../include/NcxxUtils/NcxxByte.hh \
	../include/NcxxUtils/NcxxChar.hh \
	../include/NcxxUtils/NcxxCheck.hh \
	../include/NcxxUtils/NcxxCompoundType.hh \
	../include/NcxxUtils/NcxxDim.hh \
	../include/NcxxUtils/NcxxDouble.hh \
	../include/NcxxUtils/NcxxEnumType.hh \
	../include/NcxxUtils/NcxxErrStr.hh \
	../include/NcxxUtils/NcxxException.hh \
	../include/NcxxUtils/NcxxFile.hh \
	../include/NcxxUtils/NcxxFloat.hh \
	../include/NcxxUtils/NcxxGroup.hh \
	../include/NcxxUtils/NcxxGroupAtt.hh \
	../include/NcxxUtils/NcxxInt.hh \
	../include/NcxxUtils/NcxxInt64.hh \
	../include/NcxxUtils/NcxxOpaqueType.hh \
	../include/NcxxUtils/NcxxShort.hh \
	../include/NcxxUtils/NcxxString.hh \
	../include/NcxxUtils/NcxxType.hh \
	../include/NcxxUtils/NcxxUbyte.hh \
	../include/NcxxUtils/NcxxUint.hh \
	../include/NcxxUtils/NcxxUint64.hh \
	../include/NcxxUtils/NcxxUshort.hh \
	../include/NcxxUtils/NcxxVar.hh \
	../include/NcxxUtils/NcxxVarAtt.hh \
	../include/NcxxUtils/NcxxVlenType.hh

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

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

TARGET_FILE = ../libNcxx.a

LOC_INCLUDES = -I../include $(NETCDF4_INCS)
LOC_CFLAGS = 

HDRS = \
	../include/Ncxx/Ncxx.hh \
	../include/Ncxx/NcxxAtt.hh \
	../include/Ncxx/NcxxByte.hh \
	../include/Ncxx/NcxxChar.hh \
	../include/Ncxx/NcxxCheck.hh \
	../include/Ncxx/NcxxCompoundType.hh \
	../include/Ncxx/NcxxDim.hh \
	../include/Ncxx/NcxxDouble.hh \
	../include/Ncxx/NcxxEnumType.hh \
	../include/Ncxx/NcxxErrStr.hh \
	../include/Ncxx/NcxxException.hh \
	../include/Ncxx/NcxxFile.hh \
	../include/Ncxx/NcxxFloat.hh \
	../include/Ncxx/NcxxGroup.hh \
	../include/Ncxx/NcxxGroupAtt.hh \
	../include/Ncxx/NcxxInt.hh \
	../include/Ncxx/NcxxInt64.hh \
	../include/Ncxx/NcxxOpaqueType.hh \
	../include/Ncxx/NcxxShort.hh \
	../include/Ncxx/NcxxString.hh \
	../include/Ncxx/NcxxType.hh \
	../include/Ncxx/NcxxUbyte.hh \
	../include/Ncxx/NcxxUint.hh \
	../include/Ncxx/NcxxUint64.hh \
	../include/Ncxx/NcxxUshort.hh \
	../include/Ncxx/NcxxVar.hh \
	../include/Ncxx/NcxxVarAtt.hh \
	../include/Ncxx/NcxxVlenType.hh

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

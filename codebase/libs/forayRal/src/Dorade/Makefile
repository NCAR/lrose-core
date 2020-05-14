# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
# ** Copyright UCAR (c) 2008 
# ** University Corporation for Atmospheric Research(UCAR) 
# ** National Center for Atmospheric Research(NCAR) 
# ** Research Applications Program(RAP) 
# ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA 
# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 

include $(LROSE_MAKE_INC_DIR)/lrose_make_macros

LOC_INCLUDES = -I../include $(NETCDF4_INCS)
LOC_CFLAGS =

TARGET_FILE = ../libforayRal.a

#
# file lists
#

HDRS = 

CPP_SRCS = \
	Dorade.cpp \
	DoradeBlock.cpp \
	DoradeBlockAsib.cpp \
	DoradeBlockCelv.cpp \
	DoradeBlockCfac.cpp \
	DoradeBlockCsfd.cpp \
	DoradeBlockCspd.cpp \
	DoradeBlockFrad.cpp \
	DoradeBlockFrib.cpp \
	DoradeBlockIndf.cpp \
	DoradeBlockNdds.cpp \
	DoradeBlockNull.cpp \
	DoradeBlockParm.cpp \
	DoradeBlockRadd.cpp \
	DoradeBlockRdat.cpp \
	DoradeBlockRktb.cpp \
	DoradeBlockRyib.cpp \
	DoradeBlockSitu.cpp \
	DoradeBlockSswb.cpp \
	DoradeBlockSwib.cpp \
	DoradeBlockTime.cpp \
	DoradeBlockUnknown.cpp \
	DoradeBlockVold.cpp \
	DoradeBlockWave.cpp \
	DoradeBlockXstf.cpp \
	DoradeFile.cpp \
	DoradeFileName.cpp \
	DoradeFile_WriteMethods.cpp \
	DoradeFortranBinary.cpp

#
# general targets
#

include $(LROSE_MAKE_INC_DIR)/lrose_make_lib_module_targets

#
# local targets
#

depend: depend_generic

# DO NOT DELETE THIS LINE -- make depend depends on it.

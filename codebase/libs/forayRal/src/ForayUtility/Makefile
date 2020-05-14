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
	Buffer.cpp \
	Decoder.cpp \
	Encoder.cpp \
	Fault.cpp \
	FilePath.cpp \
	FortranBinary.cpp \
	KeyAccess.cpp \
	RaycAngle.cpp \
	RaycTime.cpp

#
# general targets
#

include $(LROSE_MAKE_INC_DIR)/lrose_make_lib_module_targets

#
# local targets
#

depend: depend_generic

# DO NOT DELETE THIS LINE -- make depend depends on it.

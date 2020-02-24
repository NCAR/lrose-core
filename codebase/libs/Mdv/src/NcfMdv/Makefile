# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
# Copyright, University Corporation for Atmospheric Research (UCAR) 2009-2017. 
# The Government's right to use this data and/or software is restricted per 
# the terms of Cooperative Agreement between UCAR and the National  Science 
# Foundation, to government use only which includes the nonexclusive, 
# nontransferable, irrevocable, royalty-free license to exercise or have 
# exercised for or on behalf of the U.S. Government throughout the world. 
# All other rights are reserved. 
# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 

include $(RAP_MAKE_INC_DIR)/rap_make_macros

LOC_INCLUDES = -I../include $(NETCDF4_INCS)
MY_CFLAGS = -DNO_RADX_DATA

TARGET_FILE = ../libMdv.a

#
# file lists
#

HDRS = 

CPPC_SRCS = \
	Mdv2NcfTrans.cc \
	Ncf2MdvField.cc \
	Ncf2MdvTrans.cc \
	NcfFieldData.cc \
	NcfGridInfo.cc \
	NcfMdv.cc \
	NcfVlevelInfo.cc

#
# general targets
#

include $(RAP_MAKE_INC_DIR)/rap_make_lib_module_targets

#
# local targets
#

depend: depend_generic

# DO NOT DELETE THIS LINE -- make depend depends on it.

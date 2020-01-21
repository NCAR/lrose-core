###########################################################################
#
# Makefile for CartSim
#
###########################################################################

include $(RAP_MAKE_INC_DIR)/rap_make_macros
MY_CPPCHECKFLAGS = -E Params.cc -E Params.hh -AddStyleFAA -exclude-rules="4.16,4.09,4.14,4.12"
LOC_INC_DIR = . $(NETCDF4_INCS)
MODULE_TYPE=progcpp
TARGET_FILE= CartSim
LOC_INCLUDES = -I. -I$(LOC_INC_DIR)
LOC_CFLAGS = 
LOC_CPPC_CFLAGS = 
LOC_LDFLAGS = $(NETCDF4_LIBS) $(HDF5_LDFLAGS) -lhdf5_cpp

LOC_LIBS = -lRadx -lradar \
	   -ldsdata -lSpdb -lMdv -ldsserver -ldidss -lrapformats -leuclid \
	   -lrapmath -ltoolsa -ldataport -ltdrp $(NETCDF4_LIB_LIST) -lm
 
HDRS = 

CPPC_SRCS = \
	$(PARAMS_CC) \
	MainCartSim.cc \
	CartSimMgr.cc \
	CartSim.cc \
	CartesianSpeck.cc \
	Clutter.cc \
	Data.cc \
	DataGrid.cc \
	DataHandler.cc \
	DataHandlerCfRadial.cc \
	DataHandlerMdv.cc \
	DeviantRay.cc \
	Fields.cc \
	GustFront.cc \
	Info.cc \
	Line.cc \
	LineGustFront.cc \
	LineAndPoint.cc \
	Microburst.cc \
	OutlierData.cc \
	PolarSpeck.cc \
	Storm.cc \
	Thing.cc \
	Turbulence.cc \
	VelCircle.cc \
	Xyz.cc
#
# tdrp support
#
include $(RAP_MAKE_INC_DIR)/rap_make_tdrp_macros

#
# Standard c++ targets
#
include $(RAP_MAKE_INC_DIR)/rap_make_c++_targets

#
# tdrp targets
#

include $(RAP_MAKE_INC_DIR)/rap_make_tdrp_c++_targets

#
# local targets
#

# DO NOT DELETE THIS LINE -- make depend depends on it.

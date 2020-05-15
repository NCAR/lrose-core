###########################################################################
#
# Makefile for CartSim
#
###########################################################################

include $(LROSE_CORE_DIR)/build/make_include/lrose_make_macros

TARGET_FILE = CartSim

LOC_INCLUDES = $(NETCDF4_INCS)

LOC_LDFLAGS = $(NETCDF4_LDFLAGS)

LOC_LIBS = \
          -ldsdata -lradar -lFmq -lSpdb -lMdv  -ldsserver -ldidss \
          -lRadx -lNcxx -lrapformats -leuclid -lrapmath \
          -lphysics -ltoolsa -ldataport -ltdrp \
          $(NETCDF4_LIBS) \
          -lfftw3 -lbz2 -lz -lpthread
 
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
include $(LROSE_CORE_DIR)/build/make_include/lrose_make_tdrp_macros

#
# Standard c++ targets
#
include $(LROSE_CORE_DIR)/build/make_include/lrose_make_c++_targets

#
# tdrp targets
#

include $(LROSE_CORE_DIR)/build/make_include/lrose_make_tdrp_c++_targets

#
# local targets
#

# DO NOT DELETE THIS LINE -- make depend depends on it.

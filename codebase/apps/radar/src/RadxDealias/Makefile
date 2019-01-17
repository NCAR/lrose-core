###########################################################################
#
# Makefile for JamesDealias program
#
###########################################################################

include $(RAP_MAKE_INC_DIR)/rap_make_macros

TARGET_FILE = RadxDealias

LOC_INCLUDES = $(NETCDF4_INCS)

LOC_LIBS = -lSpdb -lrapmath -lFmq -ldsserver \
	-lrapformats -ldidss -leuclid -ltoolsa -ldataport \
	-ltdrp $(NETCDF4_LIBS) -lbz2 \
	-lz -lm

LOC_LDFLAGS = $(NETCDF4_LDFLAGS)

LOC_CFLAGS =

HDRS = \
	$(PARAMS_HH) \
	Rsl.hh \
	Args.hh \
	RadxDealias.hh \
	Dsr2Radar.hh \
	FourDD.hh 

CPPC_SRCS = \
	$(PARAMS_CC) \
	Rsl.cc \
	Args.cc \
	RadxDealias.cc \
	Dsr2Radar.cc \
	FourDD.cc \
	Main.cc 

#
# tdrp macros
#

include $(RAP_MAKE_INC_DIR)/rap_make_tdrp_macros

#
# standard C++ targets
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


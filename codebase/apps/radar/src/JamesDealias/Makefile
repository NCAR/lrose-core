###########################################################################
#
# Makefile for JamesDealias program
#
###########################################################################

include $(LROSE_MAKE_INC_DIR)/lrose_make_macros

TARGET_FILE = JamesDealias

LOC_INCLUDES = $(NETCDF4_INCS)

LOC_LIBS = -lSpdb -lrapmath -lFmq -ldsserver \
	-lrapformats -ldidss -leuclid -ltoolsa -lpthread -ldataport \
	-ltdrp -ltrmm_rsl $(NETCDF4_LIBS) -lbz2 \
	-lz -lm

LOC_LDFLAGS = $(NETCDF4_LDFLAGS)

LOC_CFLAGS =

HDRS = \
	$(PARAMS_HH) \
	Args.hh \
	JamesDealias.hh \
	Dsr2Radar.hh \
	FourDD.hh 

CPPC_SRCS = \
	$(PARAMS_CC) \
	Args.cc \
	JamesDealias.cc \
	Dsr2Radar.cc \
	FourDD.cc \
	Main.cc 

#
# tdrp macros
#

include $(LROSE_MAKE_INC_DIR)/lrose_make_tdrp_macros

#
# standard C++ targets
#

include $(LROSE_MAKE_INC_DIR)/lrose_make_c++_targets
#
# tdrp targets
#

include $(LROSE_MAKE_INC_DIR)/lrose_make_tdrp_c++_targets

#
# local targets
#

# DO NOT DELETE THIS LINE -- make depend depends on it.


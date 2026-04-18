###########################################################################
#
# Makefile for HailKEswath program
#
# Mike Dixon, RAP, NCAR, Boulder, CO, 80307, USA
#
# Sept 1998
#
###########################################################################

include $(LROSE_CORE_DIR)/build/make_include/lrose_make_macros

TARGET_FILE = HailKEswath

LOC_INCLUDES =
LOC_CFLAGS =
LOC_LDFLAGS =
LOC_LIBS = \
	-lMdv -lrapformats -ldsserver -ldidss \
	-leuclid -lrapmath -ltoolsa -ldataport \
	-ltdrp -lRadx -lNcxx -lphysics \
	$(NETCDF4_LIBS) -lbz2 -lz -lpthread

HDRS = \
	Params.hh \
	MaxData.hh \
	Args.hh \
	MethodDriver.hh \
	OutputFile.hh \
	HailKEswath.hh \
	TodPeriod.hh \
	Trigger.hh

CPPC_SRCS = \
	Params.cc \
	MaxData.cc \
	Args.cc \
	Main.cc \
	MethodDriver.cc \
	OutputFile.cc \
	HailKEswath.cc \
	RunningDriver.cc \
	SingleFileDriver.cc \
	TodDriver.cc \
	TotalDriver.cc \
	TodPeriod.cc \
	Trigger.cc

#
# tdrp macros
#

include $(LROSE_CORE_DIR)/build/make_include/lrose_make_tdrp_macros

#
# standard C++ targets
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

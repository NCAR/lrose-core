###########################################################################
#
# Makefile
#
# Mike Dixon, RAP, NCAR, Boulder, Co, USA, 80307
#
# Jan 2008
#
###########################################################################

include $(LROSE_CORE_DIR)/build/make_include/lrose_make_macros

LOC_INCLUDES = -I../include -I../include/radar
LOC_CFLAGS =

TARGET_FILE = ../libradar.a

#
# file lists
#

HDRS =

CPPC_SRCS = \
	NcarPidParams.cc \
	NcarParticleId.cc \
	PidImapManager.cc \
	PidInterestMap.cc \
	TempProfile.cc

#
# general targets
#

include $(LROSE_CORE_DIR)/build/make_include/lrose_make_lib_module_targets

#
# local targets
#

NcarPidParams.cc: paramdef.NcarPid
	tdrp_gen -f paramdef.NcarPid -c++ -class NcarPidParams -lib radar -add_ncar_copyright
	/bin/mv -f NcarPidParams.hh ../include/radar

clean_tdrp:
	$(RM) NcarPidParams.cc ../include/radar/NcarPidParams.hh

depend: depend_generic

# DO NOT DELETE THIS LINE -- make depend depends on it.

###########################################################################
#
# Makefile for simulator
# Converted from John Hubbert simulator with help from ChatGpt.
#
# Mike Dixon, EOL, NCAR, Boulder, Co, USA, 80307
#
# Sept 2026
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
	RadarTimeSeriesSim.cc

#
# general targets
#

include $(LROSE_CORE_DIR)/build/make_include/lrose_make_lib_module_targets

#
# local targets
#

depend: depend_generic

#
# testing
#

TEST_PROG = test_RadarTimeSeriesSim
TEST_OBJS = RadarTimeSeriesSim.o test_RadarTimeSeriesSim.o

test:
	$(MAKE) _CC="$(CC)" \
	DBUG_OPT_FLAGS="$(DEBUG_FLAG)" $(TEST_PROG)

$(TEST_PROG): $(TEST_OBJS)
	$(CPPC) $(DEBUG_FLAG) $(TEST_OBJS) \
	$(LDFLAGS) -o $(TEST_PROG) -lradar -ltoolsa -lm -lfftw3 $(SYS_LIBS)

# DO NOT DELETE THIS LINE -- make depend depends on it.

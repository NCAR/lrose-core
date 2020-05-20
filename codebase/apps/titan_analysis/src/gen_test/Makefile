###########################################################################
#
# Makefile for gen_test programs
#
# Mike Dixon, RAP, NCAR, Boulder, CO, 80307, USA
#
# Nov 1994
#
###########################################################################

include $(LROSE_CORE_DIR)/build/make_include/lrose_make_macros

TARGET_FILE = ascii_to_xgraph

LOC_INCLUDES =
LOC_CFLAGS =
LOC_LDFLAGS =
LOC_LIBS = \
	-ltitan -lrapmath -lmdv -lrapformats \
	-ltoolsa -lpthread -ldataport $(DEBUG_LIBS) -lbz2 \
	-lz -lm

MODULE_TYPE=program

RFUTILHDR = $(INCDIR)/titan/file_io.h
RADARHDR = $(INCDIR)/titan/radar.h
UTILHDR = $(INCDIR)/toolsa/umisc.h

SRCS = \
	gen_cart_test.c \
	gen_geom_plot.c \
	gen_radar_test.c \
	calc_range.c

RADAR_TARGET = gen_radar_test
CART_TARGET = gen_cart_test
GEOM_TARGET = gen_geom_plot
CALC_TARGET = calc_range

TARGET = $(RADAR_TARGET) $(CART_TARGET) $(GEOM_TARGET) $(CALC_TARGET)

all: $(TARGET)

$(RADAR_TARGET): gen_radar_test.o $(LIBFILES)
	$(CC) $(INCLUDES) -o $(RADAR_TARGET) gen_radar_test.o \
	$(DBUG_OPT_FLAGS) $(LDFLAGS) $(LIBS)

$(CART_TARGET): gen_cart_test.o $(LIBFILES)
	$(CC) $(INCLUDES) -o $(CART_TARGET) gen_cart_test.o \
	$(DBUG_OPT_FLAGS) $(LDFLAGS) $(LIBS)

$(GEOM_TARGET): gen_geom_plot.o $(LIBFILES)
	$(CC) $(INCLUDES) -o $(GEOM_TARGET) gen_geom_plot.o \
	$(DBUG_OPT_FLAGS) $(LDFLAGS) $(LIBS)

$(CALC_TARGET): calc_range.o $(LIBFILES)
	$(CC) $(INCLUDES) -o $(CALC_TARGET) calc_range.o \
	$(DBUG_OPT_FLAGS) $(LDFLAGS) $(LIBS)

debug:
profile:
pure:
opt:
install_bin:
install:
install_shared:

depend:
	makedepend -- $(CFLAGS) -- $(INCLUDES) *.c

clean_depend:
	makedepend

clean_xdr::

clean_tdrp::

clean_bin::
	$(RM) $(TARGET)

clean:
	$(RM) core a.out $(COPYR_CHECK)
	$(RM) *.i *.o  *.ln *~

clean_all: clean clean_bin clean_tdrp

copyright:
	copyright_remove $(SRCS) $(LOCAL_HDRS)
	copyright_prepend $(SRCS) $(LOCAL_HDRS) 

rcs_keys:
	rcs_keys_remove $(SRCS) $(LOCAL_HDRS)
	rcs_keys_prepend $(SRCS) $(LOCAL_HDRS)

tags:
	etags -t $(SRCS) $(LOCAL_HDRS)

.SUFFIXES: .c .ln .o

.c.ln:
	$(LINT) $(LINTFLAGS) -c $(INCLUDES) $< | $(LINTFILTER)

.c.o:
	$(CC) $(CFLAGS) -c $(INCLUDES) $<

# DO NOT DELETE THIS LINE -- make depend depends on it.


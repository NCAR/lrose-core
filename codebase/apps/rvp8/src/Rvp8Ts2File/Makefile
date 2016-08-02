###########################################################################
#
# Makefile for Rvp8Ts2File program
#
# Mike Dixon, RAP, NCAR
# POBox 3000, Boulder, CO, 80307, USA
#
# April 2006
#
###########################################################################

TARGET_FILE = Rvp8Ts2File
INSTALL_PATH = $(RVP8_BIN_DIR)/$(TARGET_FILE)

SHELL = /bin/sh
RM = /bin/rm -f
MV = /bin/mv -f
CP = /bin/cp
CP_DIR = /bin/cp -r
MKDIR = /bin/mkdir -p
TEST = /usr/bin/test
PERL = local/bin/perl
AWK = /usr/bin/awk

C++ = g++
MACH_OS = LINUX
INCLUDES = -I/usr/sigmet/include -I$(RVP8_INC_DIR)

DEBUG_FLAGS = -g
OPT_FLAGS = -O

LOCAL_CFLAGS =
#LOCAL_CFLAGS = -DRVP8_LEGACY_V8

LDFLAGS = -L/usr/sigmet/rda/lib -L/usr/sigmet/libs/lib \
	-L/usr/sigmet/lib -L$(RVP8_LIB_DIR)
LIBS = -lrvp8_rap -lrdasubs -ldspi -lantenna -lconfig -luser 

SRCS = \
	Args.cc \
	Main.cc \
	Rvp8Ts2File.cc

OBJS = $(SRCS:.cc=.o)

all:
	$(MAKE) CFLAGS="$(LOCAL_CFLAGS) $(DEBUG_FLAGS)" $(TARGET_FILE)

opt:
	$(MAKE) CFLAGS="$(LOCAL_CFLAGS) $(OPT_FLAGS)" $(TARGET_FILE)

static:
	$(MAKE) CFLAGS="$(LOCAL_CFLAGS) $(DEBUG_FLAGS) -static" $(TARGET_FILE)

staticopt:
	$(MAKE) CFLAGS="$(LOCAL_CFLAGS) $(OPT_FLAGS) -static" $(TARGET_FILE)

install: $(INSTALL_PATH)

$(INSTALL_PATH): all
	if $(TEST) -d $(RVP8_BIN_DIR); then :; else \
		$(MKDIR) $(RVP8_BIN_DIR); fi ; \
	if $(TEST) -f $(INSTALL_PATH) ;\
	then \
	 	$(MV) $(RVP8_BIN_DIR)/$(TARGET_FILE) $(RVP8_BIN_DIR)/$(TARGET_FILE).bak ;\
		$(CP) $(TARGET_FILE) $(INSTALL_PATH) ;\
		$(RM) $(RVP8_BIN_DIR)/$(TARGET_FILE).bak ;\
	else \
		$(CP) $(TARGET_FILE) $(INSTALL_PATH); \
	fi

clean:
	$(RM) core a.out *.o *~ junk*

clean_bin:
	$(RM) $(TARGET_FILE)

clean_all: clean_bin clean

$(TARGET_FILE): $(OBJS)
	$(C++) $(CFLAGS) -o $(TARGET_FILE)  $(OBJS) $(LDFLAGS) $(LIBS)

.SUFFIXES: .cc

.cc.o:
	$(C++) $(CFLAGS) -c $(INCLUDES) -D${MACH_OS} -DSYSV $<


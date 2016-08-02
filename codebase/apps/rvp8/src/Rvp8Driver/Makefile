###########################################################################
#
# Makefile for Rvp8Driver
#
# Mike Dixon, RAP, NCAR, Boulder, CO, 80307, USA
#
# March 2006
#
###########################################################################

TARGET_FILE = Rvp8Driver
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

CC = gcc
CPPC = g++
MACH_OS = LINUX
INCLUDES = -I/usr/sigmet/include -I/usr/sigmet/src/libs/antenna \
		-I$(RVP8_INC_DIR)

LOCAL_CFLAGS =
#LOCAL_CFLAGS = -DRVP8_LEGACY_V8

DEBUG_FLAGS = -g
OPT_FLAGS = -O

LDFLAGS = -L/usr/sigmet/rda/lib -L/usr/sigmet/libs/lib \
	-L/usr/sigmet/lib -L$(RVP8_LIB_DIR)
LIBS = -lrvp8_rap -lrdasubs -ldsp -ldspi -lantenna -lconfig -luser -lz

SRCS = \
	Args.cc \
	Commands.cc \
	DspDriver.cc \
	Main.cc \
	Rvp8Driver.cc \
	TsStatus.cc

# NOTE: local copy of /usr/sigmet/src/libs/iant_pwrp.c
#       so that we can modify the code not to exit if
#       it AntPowerUpMain things the antenna config has changed.
#       RVP8 does not need the antenna anyway

C_SRCS = \
	iant_pwrp.c

OBJS = $(SRCS:.cc=.o) $(C_SRCS:.c=.o)

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
	$(CPPC) $(CFLAGS) -o $(TARGET_FILE)  $(OBJS) $(LDFLAGS) $(LIBS)

.SUFFIXES: .cc

.cc.o:
	$(CPPC) $(CFLAGS) -c $(INCLUDES) -D${MACH_OS} -DSYSV $<

.c.o:
	$(CC) $(CFLAGS) -c $(INCLUDES) -D${MACH_OS} -DSYSV $<


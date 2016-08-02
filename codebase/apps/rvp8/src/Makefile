#
# Recursive make - makes the subdirectory code
#

#
# system utilities
#

SHELL = /bin/sh
RM = /bin/rm -f
MV = /bin/mv -f
CP = /bin/cp
CP_DIR = /bin/cp -r
MKDIR = /bin/mkdir -p
TEST = /usr/bin/test

#
# Archiving libs
#

AR = ar
ARFLAGS = rv
RANLIB = na
HAVE_RANLIB=FALSE

#
# target lists
#

TARGETS = \
	all debug opt dbgopt static staticopt \
	install \
	clean clean_bin clean_all

SUB_DIRS =  \
	Rvp8Driver \
	Rvp8Ts2File \
	TsGrabber \
	Rvp8TsTcpServer \
	Rvp8TsUdp2File

$(TARGETS): FORCE
	$(MAKE) $(SUB_DIRS) TARGET=$@

$(SUB_DIRS): FORCE
	if $(TEST) -d $@; then cd $@; $(MAKE) $(TARGET); fi ;

FORCE:



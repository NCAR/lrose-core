# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
# ** Copyright UCAR (c) 1992 - 2012 
# ** University Corporation for Atmospheric Research(UCAR) 
# ** National Center for Atmospheric Research(NCAR) 
# ** Research Applications Laboratory(RAL) 
# ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA 
# ** 2012/9/18 21:4:39 
# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
###########################################################################
#
# Makefile for ColorMap classes
#
# Mike Dixon, EOL, NCAR, Boulder, Co, USA, 80307
#
# July 2022
#
###########################################################################

include $(LROSE_CORE_DIR)/build/make_include/lrose_make_macros

LOC_INCLUDES = -I../include $(QT_INCLUDES)
LOC_CFLAGS = $(QT_LDFLAGS)

LOC_LIBS = -lSpdb -lMdv -lradar -lrapformats -lFmq -lRadx -lSolo -lNcxx \
	-ldsserver -ldidss -leuclid -lphysics \
	-lrapmath -ltoolsa -ldataport -ltdrp \
	$(QT_LIBS)  \
	-lz -lpthread -lm 

LOC_LDFLAGS = -L$(LROSE_INSTALL_DIR)/lib $(QT_LDFLAGS) 

LOC_CPPC_CFLAGS = -g  $(QT_CPPC_CFLAGS)


TARGET_FILE = ../libqtplot.a

HDRS = \
	../include/qtplot/ColorMap.hh \
	../include/qtplot/ColorBar.hh \
	../include/qtplot/X11ColorMap.hh

CPPC_SRCS = \
	ColorMap.cc \
	ColorBar.cc \
	X11ColorMap.cc

#
# general targets
#

include $(LROSE_CORE_DIR)/build/make_include/lrose_make_lib_module_targets

#
# local targets
#

depend: depend_generic

# DO NOT DELETE THIS LINE -- make depend depends on it.

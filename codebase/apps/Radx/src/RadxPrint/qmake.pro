# packages needed
#  open gl
#  libqt4-opengl-dev
#  libglut-dev

TEMPLATE = app
TARGET = RadxPrint
# ICON = HawkEyePolarIcon.icns 

CONFIG += qt
CONFIG += debug

RESOURCES = resources.qrc 

HEADERS += Args.hh
HEADERS += RadxPrint.hh
HEADERS += Params.hh

SOURCES += Args.cc
SOURCES += RadxPrint.cc
SOURCES += Params.cc
SOURCES += Main.cc

DEFINES += _BSD_TYPES
DEFINES += F_UNDERSCORE2
DEFINES += _LARGEFILE_SOURCE
DEFINES += _FILE_OFFSET_BITS=64

INCLUDEPATH += $(HOME)/rap/include
INCLUDEPATH += $(RAP_INC_DIR)

LIBS += -L$(HOME)/lrose/lib
LIBS += -L$(RAP_LIB_DIR)
LIBS += -lSpdb
LIBS += -lFmq
LIBS += -ldsserver
LIBS += -ldidss
LIBS += -leuclid
LIBS += -lphysics
LIBS += -lrapmath
LIBS += -lMdv
LIBS += -lRadx
LIBS += -lradar
LIBS += -lrapformats
LIBS += -ltoolsa
LIBS += -ldataport
LIBS += -ltdrp

LIBS += -lNcxx 
LIBS += -lnetcdf_c++ 
LIBS += -lnetcdf 
LIBS += -lhdf5_cpp 
LIBS += -lhdf5_hl 
LIBS += -lhdf5 
LIBS += -lz 
LIBS += -ludunits2 
LIBS += -lbz2 
LIBS += -lexpat 
# LIBS += -lfftw3 
LIBS += -lpthread 
LIBS += -lm  

QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.13

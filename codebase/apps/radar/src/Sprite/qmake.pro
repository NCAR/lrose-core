# packages needed
#  libqt5

TEMPLATE = app
TARGET = Sprite
ICON = SpritePolarIcon.icns 

CONFIG += qt
CONFIG += debug

RESOURCES = resources.qrc 

HEADERS += AScopeManager.hh
HEADERS += AScopeWidget.hh
HEADERS += AllocCheck.hh
HEADERS += Args.hh
HEADERS += ColorMap.hh
HEADERS += DisplayManager.hh
HEADERS += DisplayWidget.hh
HEADERS += Params.hh
HEADERS += ScaledLabel.hh
HEADERS += Sprite.hh
HEADERS += TsReader.hh
HEADERS += WorldPlot.hh

SOURCES += AScopeManager.cc
SOURCES += AScopeWidget.cc
SOURCES += AllocCheck.cc
SOURCES += Args.cc
SOURCES += ColorMap.cc
SOURCES += DisplayManager.cc
SOURCES += DisplayWidget.cc
SOURCES += Main.cc
SOURCES += Params.cc
SOURCES += ScaledLabel.cc
SSOURCES += prite.cc
SOURCES += TsReader.cc
SOURCES += WorldPlot.cc

DEFINES += _BSD_TYPES
DEFINES += F_UNDERSCORE2
DEFINES += _LARGEFILE_SOURCE
DEFINES += _FILE_OFFSET_BITS=64

INCLUDEPATH += $(RAP_INC_DIR)
# INCLUDEPATH += /usr/include/qt4/QtDesigner

LIBS += -L$(HOME)/lrose/lib
LIBS += -L$(RAP_LIB_DIR)
LIBS += -lSpdb
LIBS += -lFmq
LIBS += -ldsserver
LIBS += -ldidss
LIBS += -leuclid
LIBS += -lphysics
LIBS += -lrapmath
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
LIBS += -lfftw3 
LIBS += -lpthread 
LIBS += -lm  
LIBS += -framework QtWidgets

QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.13

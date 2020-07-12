# packages needed
#  open gl
#  libqt4-opengl-dev
#  libglut-dev

TEMPLATE = app
TARGET = HawkEyeGl

CONFIG += qt
CONFIG += debug

HEADERS += Params.hh
HEADERS += Args.hh
HEADERS += HawkEyeGl.hh
HEADERS += PPI.hh
HEADERS += ScaledLabel.hh
HEADERS += ColorMap.hh
HEADERS += ColorBar.hh
HEADERS += Reader.hh

SOURCES += Params.cc
SOURCES += Args.cc
SOURCES += Main.cc
SOURCES += HawkEyeGl.cc
SOURCES += PPI.cc
SOURCES += ScaledLabel.cc
SOURCES += ColorMap.cc
SOURCES += ColorBar.cc
SOURCES += Reader.cc

DEFINES += LINUX
DEFINES += _BSD_TYPES
DEFINES += F_UNDERSCORE2
DEFINES += _LARGEFILE_SOURCE
DEFINES += _FILE_OFFSET_BITS=64

INCLUDEPATH += $(HOME)/rap/include
INCLUDEPATH += $(RAP_INC_DIR)
INCLUDEPATH += /usr/lib/qt4/include
INCLUDEPATH += /usr/lib/qt3-3/include
INCLUDEPATH += /usr/include/qt4/QtOpenGL
INCLUDEPATH += /usr/include/qt4/QtDesigner

LIBS += -L$(HOME)/rap/lib
LIBS += -L$(RAP_LIB_DIR)
LIBS += -L/usr/lib/qt4/lib
LIBS += -L/usr/lib/qt3-3/lib
LIBS += -lFmq
LIBS += -ldsserver
LIBS += -ldidss
LIBS += -lRadx
LIBS += -lradar
LIBS += -lrapformats
LIBS += -ltoolsa
LIBS += -ldataport
LIBS += -ltdrp
LIBS += -lQtOpenGL
LIBS += -lGL
LIBS += -lglut




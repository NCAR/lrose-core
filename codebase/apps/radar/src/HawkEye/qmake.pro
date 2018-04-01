# packages needed
#  open gl
#  libqt4-opengl-dev
#  libglut-dev

TEMPLATE = app
TARGET = HawkEye

CONFIG += qt
CONFIG += debug

HEADERS += Args.hh
HEADERS += BscanManager.hh
HEADERS += BscanWidget.hh
HEADERS += ColorBar.hh
HEADERS += ColorMap.hh
HEADERS += FieldRenderer.hh
HEADERS += DisplayManager.hh
HEADERS += HawkEye.hh
HEADERS += Params.hh
HEADERS += PolarManager.hh
HEADERS += PolarWidget.hh
HEADERS += PpiWidget.hh
HEADERS += RhiWidget.hh
HEADERS += RhiWindow.hh
HEADERS += ScaledLabel.hh
HEADERS += TimeScaleWidget.hh

SOURCES += Args.cc
SOURCES += BscanManager.cc
SOURCES += BscanWidget.cc
SOURCES += ColorBar.cc
SOURCES += ColorMap.cc
SOURCES += FieldRenderer.cc
SOURCES += DisplayManager.cc
SOURCES += HawkEye.cc
SOURCES += Main.cc
SOURCES += Params.cc
SOURCES += PolarManager.cc
SOURCES += PolarWidget.cc
SOURCES += PpiWidget.cc
SOURCES += RhiWidget.cc
SOURCES += RhiWindow.cc
SOURCES += ScaledLabel.cc
SOURCES += TimeScaleWidget.cc

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




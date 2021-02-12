# qmake.pro for IpsAScope

QT += widgets
QT += qml
# requires(qtConfig(filedialog))

TEMPLATE = app
TARGET = IpsAscope
ICON = CycloneIcon.icns 

CONFIG += qt
CONFIG += debug

RESOURCES = resources.qrc 

HEADERS += AScope.hh
HEADERS += AScopeReader.hh
HEADERS += IpsAscope.hh
HEADERS += Args.hh
HEADERS += Knob.hh
HEADERS += Logger.hh
HEADERS += Params.hh
HEADERS += PlotInfo.hh
HEADERS += ScopePlot.hh
HEADERS += ScrollBar.hh
HEADERS += ScrollZoomer.hh
HEADERS += ui_AScope.hh
HEADERS += ui_Knob.hh
HEADERS += ui_ScopePlot.hh

SOURCES += AScope.cc
SOURCES += AScopeReader.cc
SOURCES += IpsAscope.cc
SOURCES += Args.cc
SOURCES += Knob.cc
SOURCES += Logger.cc
SOURCES += Main.cc
SOURCES += Params.cc
SOURCES += PlotInfo.cc
SOURCES += ScopePlot.cc
SOURCES += ScrollBar.cc
SOURCES += ScrollZoomer.cc

DEFINES += _BSD_TYPES
DEFINES += F_UNDERSCORE2
DEFINES += _LARGEFILE_SOURCE
DEFINES += _FILE_OFFSET_BITS=64

# QMAKE_CXXFLAGS += -isystem

INCLUDEPATH += /usr/local/include /usr/local/lrose/include

LIBS += -L$(HOME)/lrose/lib
LIBS += -L/usr/local/lib
LIBS += -L/usr/local/lrose/lib
LIBS += -L$(LROSE_INSTALL_DIR)/lib

lroseDir = $$(LROSE_INSTALL_DIR)
!isEmpty(lroseDir) {
  INCLUDEPATH += $$lroseDir/include
  LIBS += $$lroseDir/lib
}

LIBS += -lFmq
LIBS += -ldsserver
LIBS += -ldidss
LIBS += -lRadx
LIBS += -lradar
LIBS += -lrapformats
LIBS += -ltoolsa
LIBS += -ldataport
LIBS += -ltdrp

LIBS += -lNcxx 
LIBS += -lnetcdf 
LIBS += -lhdf5_hl 
LIBS += -lhdf5 
LIBS += -lz 
LIBS += -lbz2 
LIBS += -lexpat 
LIBS += -lpthread 
LIBS += -lm  
LIBS += -qwt
LIBS += -framework QtWidgets
LIBS += -framework QtScript
LIBS += -framework QtQml

QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.13

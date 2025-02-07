# qmake.pro for Lucid

QT += widgets
QT += qml
# requires(qtConfig(filedialog))

TEMPLATE = app
TARGET = Lucid
ICON = LucidCycloneIcon.icns 

CONFIG += qt
CONFIG += debug

RESOURCES = resources.qrc 
# RESOURCES = customcompleter.qrc

HEADERS += Args.hh
HEADERS += GuiManager.hh
HEADERS += Cdraw_P.hh
HEADERS += Cgui_P.hh
HEADERS += Cimages_P.hh
HEADERS += Croutes_P.hh
HEADERS += Csyprod_P.hh
HEADERS += Cterrain_P.hh
HEADERS += FieldTableItem.hh
HEADERS += HorizView.hh
HEADERS += LatLonBox.hh
HEADERS += LegacyParams.hh
HEADERS += Lucid.hh
HEADERS += MapMenuItem.hh
HEADERS += MdvReader.hh
HEADERS += Params.hh
HEADERS += ProdMenuItem.hh
HEADERS += Product.hh
HEADERS += ProductMgr.hh
HEADERS += RenderContext.hh
HEADERS += ScaledLabel.hh
HEADERS += SiiPalette.hh
HEADERS += SymprodRender.hh
HEADERS += SymprodRenderObj.hh
HEADERS += TimeControl.hh
HEADERS += TimeList.hh
HEADERS += VertView.hh
HEADERS += VertManager.hh
HEADERS += VlevelManager.hh
HEADERS += WayPts.hh
HEADERS += WindMenuItem.hh
HEADERS += WorldPlot.hh
HEADERS += XyBox.hh
HEADERS += ZoomMenuItem.hh
HEADERS += cidd.h
HEADERS += cidd_contours.h
HEADERS += cidd_data_io.h
HEADERS += cidd_dpd.h
HEADERS += cidd_layers.h
HEADERS += cidd_legend.h
HEADERS += cidd_macros.h
HEADERS += cidd_maps.h
HEADERS += cidd_movies.h
HEADERS += cidd_params.h
HEADERS += cidd_products.h
HEADERS += cidd_structs.h
HEADERS += cidd_windows.h
HEADERS += cidd_winds.h

SOURCES += Args.cc
SOURCES += GuiManager.cc
SOURCES += Cdraw_P.cc
SOURCES += Cgui_P.cc
SOURCES += Cimages_P.cc
SOURCES += Croutes_P.cc
SOURCES += Csyprod_P.cc
SOURCES += Cterrain_P.cc
SOURCES += FieldTableItem.cc
SOURCES += HorizView.cc
SOURCES += LatLonBox.cc
SOURCES += LegacyParams.cc
SOURCES += Lucid.cc
SOURCES += Main.cc
SOURCES += MapMenuItem.cc
SOURCES += MdvReader.cc
SOURCES += Params.cc
SOURCES += ProdMenuItem.cc
SOURCES += Product.cc
SOURCES += ProductMgr.cc
SOURCES += RenderContext.cc
SOURCES += ScaledLabel.cc
SOURCES += SymprodRender.cc
SOURCES += SymprodRenderObj.cc
SOURCES += TimeControl.cc
SOURCES += TimeList.cc
SOURCES += VertView.cc
SOURCES += VertManager.cc
SOURCES += VlevelManager.cc
SOURCES += WayPts.cc
SOURCES += WindMenuItem.cc
SOURCES += WorldPlot.cc
SOURCES += XyBox.cc
SOURCES += ZoomMenuItem.cc

DEFINES += _BSD_TYPES
DEFINES += F_UNDERSCORE2
DEFINES += _LARGEFILE_SOURCE
DEFINES += _FILE_OFFSET_BITS=64

# QMAKE_CXXFLAGS += -isystem

INCLUDEPATH += /usr/local/include /usr/local/lrose/include
INCLUDEPATH += $(HOME)/lrose/include

LIBS += -L$(HOME)/lrose/lib
LIBS += -L/usr/local/lib
LIBS += -L/usr/local/lrose/lib

lroseDir = $$(LROSE_INSTALL_DIR)
!isEmpty(lroseDir) {
  INCLUDEPATH += $$lroseDir/include
  LIBS += $$lroseDir/lib
}

LIBS += -ldsdata
LIBS += -lradar
LIBS += -lMdv
LIBS += -lSpdb
LIBS += -lFmq
LIBS += -lrapformats
LIBS += -ldsserver
LIBS += -ldidss
LIBS += -leuclid
LIBS += -lrapmath
LIBS += -lrapplot
LIBS += -lqtplot
LIBS += -ltoolsa
LIBS += -ldataport
LIBS += -ltdrp
LIBS += -lRadx
LIBS += -lNcxx
LIBS += -lnetcdf 
LIBS += -lhdf5_hl 
LIBS += -lhdf5 
LIBS += -lphysics
LIBS += -lshapelib
LIBS += -lXext
LIBS += -lX11
LIBS += -lfftw3
LIBS += -lpng
LIBS += -lbz2
LIBS += -lz
LIBS += -lpthread
LIBS += -lexpat 
LIBS += -lm
LIBS += -framework QtWidgets
LIBS += -framework QtScript
LIBS += -framework QtQml

QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.13

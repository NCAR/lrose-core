# qmake.pro for HawkEye

QT += widgets
QT += qml
# requires(qtConfig(filedialog))

TEMPLATE = app
TARGET = HawkEye
ICON = HawkEyeCycloneIcon.icns 

CONFIG += qt
CONFIG += debug

RESOURCES = resources.qrc 
# RESOURCES = customcompleter.qrc

HEADERS += AllocCheck.hh
HEADERS += Args.hh
HEADERS += Beam.hh
HEADERS += BoundaryPointEditor.hh
HEADERS += BscanBeam.hh
HEADERS += BscanManager.hh
HEADERS += BscanWidget.hh
HEADERS += ColorBar.hh
HEADERS += ColorTableManager.hh
HEADERS += DisplayField.hh
HEADERS += DisplayManager.hh
HEADERS += FieldRenderer.hh
HEADERS += HawkEye.hh
HEADERS += HawkEyeLogger.hh
HEADERS += PaletteManager.hh
HEADERS += Params.hh
HEADERS += ParameterColorView.hh
HEADERS += FieldColorController.hh
HEADERS += DisplayFieldModel.hh
HEADERS += PpiBeam.hh
HEADERS += PolarManager.hh
HEADERS += PolarWidget.hh
HEADERS += PpiWidget.hh
HEADERS += Reader.hh
HEADERS += RhiBeam.hh
HEADERS += RhiWidget.hh
HEADERS += RhiWindow.hh
HEADERS += ScaledLabel.hh
HEADERS += SiiPalette.hh
HEADERS += SoloDefaultColorWrapper.hh
HEADERS += SweepManager.hh
HEADERS += TimeScaleWidget.hh
HEADERS += WorldPlot.hh

# for soloii editing 
HEADERS += FlowLayout.hh
# HEADERS += DialogOptionsWidget.hh
HEADERS += ColorMapTemplates.hh
HEADERS += ClickableLabel.hh
HEADERS += TextEdit.hh
HEADERS += SpreadSheetController.hh
HEADERS += SpreadSheetView.hh
HEADERS += SpreadSheetModel.hh
HEADERS += SpreadSheetItem.hh
HEADERS += SpreadSheetUtils.hh
HEADERS += SpreadSheetDelegate.hh
HEADERS += FunctionEditor.hh
HEADERS += SoloFunctions.hh
HEADERS += DataField.hh

SOURCES += AllocCheck.cc
SOURCES += Args.cc
SOURCES += Beam.cc
SOURCES += BoundaryPointEditor.cc
SOURCES += BscanBeam.cc
SOURCES += BscanManager.cc
SOURCES += BscanWidget.cc
SOURCES += ColorBar.cc
SOURCES += ColorTableManager.cc
SOURCES += FieldRenderer.cc
SOURCES += DisplayField.cc
SOURCES += DisplayManager.cc
SOURCES += HawkEye.cc
SOURCES += HawkEyeLogger.cc
SOURCES += Main.cc
SOURCES += PaletteManager.cc
SOURCES += Params.cc
SOURCES += ParameterColorView.cc
SOURCES += FieldColorController.cc
SOURCES += DisplayFieldModel.cc
SOURCES += PolarManager.cc
SOURCES += PolarWidget.cc
SOURCES += PpiBeam.cc
SOURCES += PpiWidget.cc
SOURCES += Reader.cc
SOURCES += RhiBeam.cc
SOURCES += RhiWidget.cc
SOURCES += RhiWindow.cc
SOURCES += ScaledLabel.cc
SOURCES += SiiPalette.cc
SOURCES += SoloDefaultColorWrapper.cc
SOURCES += SweepManager.cc
SOURCES += TimeScaleWidget.cc
SOURCES += FlowLayout.cc
# SOURCES += DialogOptionsWidget.cc
SOURCES += ColorMapTemplates.cc
SOURCES += ClickableLabel.cc
SOURCES += TextEdit.cc
SOURCES += SpreadSheetController.cc
SOURCES += SpreadSheetView.cc
SOURCES += SpreadSheetModel.cc
SOURCES += SpreadSheetItem.cc
SOURCES += SpreadSheetUtils.cc
SOURCES += SpreadSheetDelegate.cc
SOURCES += FunctionEditor.cc
SOURCES += SoloFunctions.cc
SOURCES += SoloFunctionsModel.cc
SOURCES += WorldPlot.cc


DEFINES += _BSD_TYPES
DEFINES += F_UNDERSCORE2
DEFINES += _LARGEFILE_SOURCE
DEFINES += _FILE_OFFSET_BITS=64

# QMAKE_CXXFLAGS += -isystem

INCLUDEPATH += /usr/local/include /usr/local/lrose/include
INCLUDEPATH += $(HOME)/rap/include

# INCLUDEPATH += $(RAP_INC_DIR)
# INCLUDEPATH += /usr/lib/qt4/include
# INCLUDEPATH += /usr/lib/qt3-3/include
# INCLUDEPATH += /usr/include/qt4/QtOpenGL
# INCLUDEPATH += /usr/include/qt4/QtDesigner

LIBS += -L$(HOME)/lrose/lib
LIBS += -L/usr/local/lib
LIBS += -L/usr/local/lrose/lib
LIBS += -L$(RAP_LIB_DIR)

lroseDir = $$(LROSE_INSTALL_DIR)
!isEmpty(lroseDir) {
  INCLUDEPATH += $$lroseDir/include
  LIBS += $$lroseDir/lib
}

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
# LIBS += -lQtOpenGL
# LIBS += -lGL
# LIBS += -lglut

LIBS += -lNcxx 
LIBS += -lnetcdf 
LIBS += -lhdf5_hl 
LIBS += -lhdf5 
LIBS += -lz 
# LIBS += -ludunits2 
LIBS += -lbz2 
LIBS += -lexpat 
# LIBS += -lfftw3 
LIBS += -lpthread 
LIBS += -lm  
LIBS += -framework QtWidgets
LIBS += -framework QtScript
LIBS += -framework QtQml

QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.13

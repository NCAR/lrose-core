# qmake.pro for IpsEye

QT += widgets
QT += qml
# requires(qtConfig(filedialog))

TEMPLATE = app
TARGET = IpsEye
ICON = CycloneIcon.icns 

CONFIG += qt
CONFIG += debug

RESOURCES = resources.qrc 

HEADERS += AllocCheck.hh
HEADERS += Args.hh
HEADERS += Beam.hh
HEADERS += BoundaryPointEditor.hh
HEADERS += BscanBeam.hh
HEADERS += BscanManager.hh
HEADERS += BscanWidget.hh
HEADERS += ColorBar.hh
HEADERS += ColorMap.hh
HEADERS += ColorTableManager.hh
HEADERS += DisplayField.hh
HEADERS += DisplayManager.hh
HEADERS += FieldRenderer.hh
HEADERS += IpsEye.hh
HEADERS += IpsEyeLogger.hh
HEADERS += PaletteManager.hh
HEADERS += Params.hh
HEADERS += ParameterColorView.hh
HEADERS += FieldColorController.hh
HEADERS += DisplayFieldModel.hh
HEADERS += PpiBeam.hh
HEADERS += PolarManager.hh
HEADERS += PolarWidget.hh
HEADERS += Reader.hh
HEADERS += RhiBeam.hh
HEADERS += ScaledLabel.hh
HEADERS += SiiPalette.hh
HEADERS += SoloDefaultColorWrapper.hh
HEADERS += SweepManager.hh
HEADERS += TimeScaleWidget.hh
HEADERS += WorldPlot.hh

# for soloii editing 
HEADERS += FlowLayout.hh
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
SOURCES += ColorMap.cc
SOURCES += ColorTableManager.cc
SOURCES += FieldRenderer.cc
SOURCES += DisplayField.cc
SOURCES += DisplayManager.cc
SOURCES += IpsEye.cc
SOURCES += IpsEyeLogger.cc
SOURCES += Main.cc
SOURCES += PaletteManager.cc
SOURCES += Params.cc
SOURCES += ParameterColorView.cc
SOURCES += FieldColorController.cc
SOURCES += DisplayFieldModel.cc
SOURCES += PolarManager.cc
SOURCES += PolarWidget.cc
SOURCES += PpiBeam.cc
SOURCES += Reader.cc
SOURCES += RhiBeam.cc
SOURCES += ScaledLabel.cc
SOURCES += SiiPalette.cc
SOURCES += SoloDefaultColorWrapper.cc
SOURCES += SweepManager.cc
SOURCES += TimeScaleWidget.cc
SOURCES += FlowLayout.cc
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

LIBS += -L$(HOME)/lrose/lib
LIBS += -L/usr/local/lib
LIBS += -L/usr/local/lrose/lib
LIBS += -L$(LROSE_INSTALL_DIR)/lib

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

LIBS += -lNcxx 
LIBS += -lnetcdf_c++ 
LIBS += -lhdf5_hl 
LIBS += -lhdf5 
LIBS += -lz 
LIBS += -lbz2 
LIBS += -lexpat 
LIBS += -lpthread 
LIBS += -lm  
LIBS += -framework QtWidgets
LIBS += -framework QtScript
LIBS += -framework QtQml

QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.13

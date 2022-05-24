QT += widgets
QT += qml
requires(qtConfig(filedialog))

CONFIG += qt debug
CONFIG += -fno-limit-debug-info

INCPATH += /Users/brenda/lrose/include

HEADERS       = Dialog.hh
HEADERS      += ../HawkEye/ColorMap.hh
HEADERS      += ../HawkEye/ColorBar.hh
HEADERS      += FlowLayout.hh
HEADERS      += ParameterColorDialog.hh
HEADERS      += DialogOptionsWidget.hh
HEADERS      += ColorMapTemplates.hh
HEADERS      += ClickableLabel.hh
HEADERS      += ../ExamineEdit/TextEdit.hh
HEADERS      += ../ExamineEdit/SpreadSheetController.hh
HEADERS      += ../ExamineEdit/SpreadSheetView.hh
HEADERS      += ../ExamineEdit/SpreadSheetModel.hh
HEADERS      += ../ExamineEdit/SpreadSheetItem.hh
HEADERS      += ../ExamineEdit/SpreadSheetUtils.hh
HEADERS      += ../ExamineEdit/spreadsheetdelegate.hh
HEADERS      += ../ExamineEdit/FunctionEditor.hh
HEADERS      += ../ExamineEdit/SoloFunctions.hh
HEADERS      += ../ExamineEdit/SoloFunctionsModel.hh
HEADERS      += ../ExamineEdit/DataField.hh
HEADERS      += ../SoloFunctionsApi/GeneralDefinitions.hh

SOURCES       = Dialog.cc 
SOURCES      += ../HawkEye/ColorMap.cc 
SOURCES      += ../HawkEye/ColorBar.cc 
SOURCES      += FlowLayout.cc 
SOURCES      += ParameterColorDialog.cc 
SOURCES      += DialogOptionsWidget.cc 
SOURCES      += ColorMapTemplates.cc 
SOURCES      += ClickableLabel.cc 
SOURCES      += ../ExamineEdit/TextEdit.cc 
SOURCES      += ../ExamineEdit/SpreadSheetController.cc
SOURCES      += ../ExamineEdit/SpreadSheetView.cc
SOURCES      += ../ExamineEdit/SpreadSheetModel.cc
SOURCES      += ../ExamineEdit/SpreadSheetItem.cc
SOURCES      += ../ExamineEdit/SpreadSheetUtils.cc
SOURCES      += ../ExamineEdit/spreadsheetdelegate.cc
SOURCES      += ../ExamineEdit/FunctionEditor.cc
SOURCES      += ../ExamineEdit/SoloFunctions.cc
SOURCES      += ../ExamineEdit/SoloFunctionsModel.cc
SOURCES      += ../SoloFunctionsApi/RemoveAcMotion.cc
SOURCES      += ../SoloFunctionsApi/AcVel.cc
SOURCES      += Main.cc

RESOURCES = customcompleter.qrc

INCLUDEPATH += $(HOME)/rap/include
INCLUDEPATH += $(RAP_INC_DIR)

LIBS += -L$(HOME)/lrose/lib
LIBS += -L$(RAP_LIB_DIR)
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
# LIBS += -lfftw3
LIBS += -lpthread
LIBS += -lm
LIBS += -framework QtWidgets
LIBS += -framework QtScript

QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.13

# install
target.path = $$[QT_INSTALL_EXAMPLES]/widgets/dialogs/standarddialogs
INSTALLS += target

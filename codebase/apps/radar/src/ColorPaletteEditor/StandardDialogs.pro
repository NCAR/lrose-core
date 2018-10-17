QT += widgets
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
HEADERS      += ../ExamineEdit/SpreadSheetController.hh
HEADERS      += ../ExamineEdit/SpreadSheetView.hh
HEADERS      += ../ExamineEdit/SpreadSheetModel.hh
HEADERS      += ../ExamineEdit/SpreadSheetItem.hh
HEADERS      += ../ExamineEdit/SpreadSheetUtils.hh
HEADERS      += ../ExamineEdit/SpreadSheetDelegate.hh

SOURCES       = Dialog.cc 
SOURCES      += ../HawkEye/ColorMap.cc 
SOURCES      += ../HawkEye/ColorBar.cc 
SOURCES      += FlowLayout.cc 
SOURCES      += ParameterColorDialog.cc 
SOURCES      += DialogOptionsWidget.cc 
SOURCES      += ColorMapTemplates.cc 
SOURCES      += ClickableLabel.cc 
SOURCES      += ../ExamineEdit/SpreadSheetController.cc
SOURCES      += ../ExamineEdit/SpreadSheetView.cc
SOURCES      += ../ExamineEdit/SpreadSheetModel.cc
SOURCES      += ../ExamineEdit/SpreadSheetItem.cc
SOURCES      += ../ExamineEdit/SpreadSheetUtils.cc
SOURCES      += ../ExamineEdit/SpreadSheetDelegate.cc
SOURCES      += Main.cc

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
LIBS += -framework QtWidgets

QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.13

# install
target.path = $$[QT_INSTALL_EXAMPLES]/widgets/dialogs/standarddialogs
INSTALLS += target

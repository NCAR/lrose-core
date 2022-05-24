QT += widgets
# requires(qtConfig(treeview))
# qtHaveModule(printsupport): QT += printsupport
#unix:qtHaveModule(dbus): QT += dbus widgets

# packages needed
#  open gl
#  libqt4-opengl-dev
#  libglut-dev

TEMPLATE = app
TARGET = ExamineEdit

CONFIG += qt
CONFIG += debug

INCPATH += /Users/brenda/lrose/include

HEADERS += spreadsheetdelegate.hh \
           spreadsheetitem.hh \
           SpreadSheetView.hh \
           SpreadSheetModel.hh \
           SpreadSheetController.hh \
           SpreadSheetUtils.hh
SOURCES += main.cc \
           spreadsheetdelegate.cc \ 
           SpreadSheetUtils.cc \
           SpreadSheetView.cc \
           SpreadSheetModel.cc \
           SpreadSheetController.cc \
           spreadsheetitem.cc
RESOURCES += spreadsheet.qrc

DEFINES += _BSD_TYPES
DEFINES += F_UNDERSCORE2
DEFINES += _LARGEFILE_SOURCE
DEFINES += _FILE_OFFSET_BITS=64

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
LIBS += -ludunits2
LIBS += -lbz2
LIBS += -lexpat
# LIBS += -lfftw3
LIBS += -lpthread
LIBS += -lm
LIBS += -framework QtWidgets

QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.13

# build_all:!build_pass {
    # CONFIG -= build_all
    # CONFIG += release
# }

# install
# target.path = $$[QT_INSTALL_EXAMPLES]/widgets/itemviews/spreadsheet
# INSTALLS += target

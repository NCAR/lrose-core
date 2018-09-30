QT += widgets
# requires(qtConfig(treeview))
# qtHaveModule(printsupport): QT += printsupport
#unix:qtHaveModule(dbus): QT += dbus widgets

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

build_all:!build_pass {
    CONFIG -= build_all
    CONFIG += release
}

# install
target.path = $$[QT_INSTALL_EXAMPLES]/widgets/itemviews/spreadsheet
INSTALLS += target

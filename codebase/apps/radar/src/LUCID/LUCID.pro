#-------------------------------------------------
#
# Project created by QtCreator 2019-07-31T16:53:02
#
#-------------------------------------------------

QT       += core gui charts

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = LUCID
TEMPLATE = app

INCLUDEPATH += /Users/katsampe/lrose/include

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

CONFIG += c++11

SOURCES += \
        contFieldDock.cpp \
        contGridConfigDialog.cpp \
        contMainImage.cpp \
        contMiscConfig.cpp \
        contOverlaysDialog.cpp \
        contPlayerDock.cpp \
        contStatusDialog.cpp \
        contToolBars.cpp \
        contValuesDisplay.cpp \
        contVsection.cpp \
        contWindDialog.cpp \
        contZoomOptions.cpp \
        main.cpp \
        mainwindow.cpp \
        viewGridConfigDialog.cpp \
        viewMainImage.cpp \
        viewMiscConfigDialog.cpp \
        viewOverlaysDialog.cpp \
        viewPlayerDock.cpp \
        viewStatusDialog.cpp \
        viewValuesDisplay.cpp \
        viewVsection.cpp \
        viewWindDialog.cpp \
        viewZoomOptions.cpp \
        Args.cc \
        Params.cc

HEADERS += \
        contFieldDock.h \
        contGridConfigDialog.h \
        contMainImage.h \
        contMiscConfig.h \
        contOverlaysDialog.h \
        contPlayerDock.h \
        contStatusDialog.h \
        contToolBars.h \
        contValuesDisplay.h \
        contVsection.h \
        contWindDialog.h \
        contZoomOptions.h \
        mainwindow.h \
        viewGridConfigDialog.h \
        viewMainImage.h \
        viewMiscConfigDialog.h \
        viewOverlaysDialog.h \
        viewPlayerDock.h \
        viewStatusDialog.h \
        viewValuesDisplay.h \
        viewVsection.h \
        viewWindDialog.h \
        viewZoomOptions.h \
        Args.hh \
        Params.hh

FORMS += \
        mainwindow.ui

LIBS += -L/Users/katsampe/lrose/lib -ltdrp

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    res.qrc

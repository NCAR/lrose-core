/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created by: Qt User Interface Compiler version 5.13.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenu>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QToolBar>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QAction *actionOpen;
    QAction *actionSave;
    QAction *actionPrint;
    QAction *actionView_Tutorial_Page;
    QAction *actionNew;
    QAction *actionRedo;
    QAction *actionCut;
    QAction *actionCopy;
    QAction *actionPaste;
    QAction *actionZoomIn;
    QAction *actionVsection;
    QAction *actionFields;
    QAction *actionMaps;
    QAction *actionProducts;
    QAction *actionData_Layers;
    QAction *actionOverlays;
    QAction *actionWind_Layer;
    QAction *actionStatus_Window;
    QAction *actionSave_Selection;
    QAction *actionReset;
    QAction *actionReload;
    QAction *actionMovie_Player;
    QAction *actionUndo;
    QAction *actionZoomOut;
    QAction *actionZoom_Window;
    QAction *actionPlay_Loop;
    QAction *actionValues_Cursor;
    QAction *actionMisc_Configuration;
    QWidget *centralWidget;
    QMenuBar *menuBar;
    QMenu *menuFile;
    QMenu *menuEdit;
    QMenu *menuView;
    QMenu *menuWindow;
    QMenu *menuShow;
    QMenu *menuZoom;
    QMenu *menuType_Here;
    QMenu *menuTools;
    QMenu *menuHelp;
    QToolBar *mainToolBar;
    QStatusBar *statusBar;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName(QString::fromUtf8("MainWindow"));
        MainWindow->setEnabled(true);
        MainWindow->resize(825, 800);
        QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
        sizePolicy.setHorizontalStretch(1);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(MainWindow->sizePolicy().hasHeightForWidth());
        MainWindow->setSizePolicy(sizePolicy);
        MainWindow->setWindowOpacity(1.000000000000000);
        MainWindow->setToolTipDuration(-6);
#if QT_CONFIG(whatsthis)
        MainWindow->setWhatsThis(QString::fromUtf8(""));
#endif // QT_CONFIG(whatsthis)
        MainWindow->setAutoFillBackground(false);
        MainWindow->setAnimated(false);
        MainWindow->setDocumentMode(false);
        actionOpen = new QAction(MainWindow);
        actionOpen->setObjectName(QString::fromUtf8("actionOpen"));
        QIcon icon;
        icon.addFile(QString::fromUtf8(":/images/openProject.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionOpen->setIcon(icon);
        actionOpen->setIconVisibleInMenu(false);
        actionSave = new QAction(MainWindow);
        actionSave->setObjectName(QString::fromUtf8("actionSave"));
        QIcon icon1;
        icon1.addFile(QString::fromUtf8(":/images/save.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionSave->setIcon(icon1);
        actionSave->setIconVisibleInMenu(false);
        actionPrint = new QAction(MainWindow);
        actionPrint->setObjectName(QString::fromUtf8("actionPrint"));
        QIcon icon2;
        icon2.addFile(QString::fromUtf8(":/images/print.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionPrint->setIcon(icon2);
        actionPrint->setIconVisibleInMenu(false);
        actionView_Tutorial_Page = new QAction(MainWindow);
        actionView_Tutorial_Page->setObjectName(QString::fromUtf8("actionView_Tutorial_Page"));
        actionNew = new QAction(MainWindow);
        actionNew->setObjectName(QString::fromUtf8("actionNew"));
        QIcon icon3;
        icon3.addFile(QString::fromUtf8(":/images/newProject.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionNew->setIcon(icon3);
        actionNew->setIconVisibleInMenu(false);
        actionRedo = new QAction(MainWindow);
        actionRedo->setObjectName(QString::fromUtf8("actionRedo"));
        QIcon icon4;
        icon4.addFile(QString::fromUtf8(":/images/images/redo.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionRedo->setIcon(icon4);
        actionRedo->setIconVisibleInMenu(false);
        actionCut = new QAction(MainWindow);
        actionCut->setObjectName(QString::fromUtf8("actionCut"));
        actionCopy = new QAction(MainWindow);
        actionCopy->setObjectName(QString::fromUtf8("actionCopy"));
        actionPaste = new QAction(MainWindow);
        actionPaste->setObjectName(QString::fromUtf8("actionPaste"));
        actionZoomIn = new QAction(MainWindow);
        actionZoomIn->setObjectName(QString::fromUtf8("actionZoomIn"));
        QIcon icon5;
        icon5.addFile(QString::fromUtf8(":/images/images/ZoomIn.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionZoomIn->setIcon(icon5);
        actionZoomIn->setIconVisibleInMenu(false);
        actionVsection = new QAction(MainWindow);
        actionVsection->setObjectName(QString::fromUtf8("actionVsection"));
        QIcon icon6;
        icon6.addFile(QString::fromUtf8(":/images/images/graph.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionVsection->setIcon(icon6);
        actionFields = new QAction(MainWindow);
        actionFields->setObjectName(QString::fromUtf8("actionFields"));
        actionFields->setCheckable(true);
        QIcon icon7;
        icon7.addFile(QString::fromUtf8(":/images/images/fields.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionFields->setIcon(icon7);
        actionMaps = new QAction(MainWindow);
        actionMaps->setObjectName(QString::fromUtf8("actionMaps"));
        actionMaps->setCheckable(true);
        actionMaps->setChecked(false);
        actionMaps->setEnabled(true);
        QIcon icon8;
        icon8.addFile(QString::fromUtf8(":/images/images/map.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionMaps->setIcon(icon8);
        actionMaps->setIconVisibleInMenu(false);
        actionProducts = new QAction(MainWindow);
        actionProducts->setObjectName(QString::fromUtf8("actionProducts"));
        actionProducts->setCheckable(true);
        QIcon icon9;
        icon9.addFile(QString::fromUtf8(":/images/images/product.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionProducts->setIcon(icon9);
        actionData_Layers = new QAction(MainWindow);
        actionData_Layers->setObjectName(QString::fromUtf8("actionData_Layers"));
        QIcon icon10;
        icon10.addFile(QString::fromUtf8(":/images/images/layers.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionData_Layers->setIcon(icon10);
        actionOverlays = new QAction(MainWindow);
        actionOverlays->setObjectName(QString::fromUtf8("actionOverlays"));
        QIcon icon11;
        icon11.addFile(QString::fromUtf8(":/images/images/outline.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionOverlays->setIcon(icon11);
        actionWind_Layer = new QAction(MainWindow);
        actionWind_Layer->setObjectName(QString::fromUtf8("actionWind_Layer"));
        QIcon icon12;
        icon12.addFile(QString::fromUtf8(":/images/images/wind.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionWind_Layer->setIcon(icon12);
        actionStatus_Window = new QAction(MainWindow);
        actionStatus_Window->setObjectName(QString::fromUtf8("actionStatus_Window"));
        QIcon icon13;
        icon13.addFile(QString::fromUtf8(":/images/images/statusWindow.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionStatus_Window->setIcon(icon13);
        actionSave_Selection = new QAction(MainWindow);
        actionSave_Selection->setObjectName(QString::fromUtf8("actionSave_Selection"));
        actionReset = new QAction(MainWindow);
        actionReset->setObjectName(QString::fromUtf8("actionReset"));
        actionReload = new QAction(MainWindow);
        actionReload->setObjectName(QString::fromUtf8("actionReload"));
        actionMovie_Player = new QAction(MainWindow);
        actionMovie_Player->setObjectName(QString::fromUtf8("actionMovie_Player"));
        actionMovie_Player->setCheckable(true);
        QIcon icon14;
        icon14.addFile(QString::fromUtf8(":/images/images/play.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionMovie_Player->setIcon(icon14);
        actionUndo = new QAction(MainWindow);
        actionUndo->setObjectName(QString::fromUtf8("actionUndo"));
        QIcon icon15;
        icon15.addFile(QString::fromUtf8(":/images/images/Undo.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionUndo->setIcon(icon15);
        actionUndo->setIconVisibleInMenu(false);
        actionZoomOut = new QAction(MainWindow);
        actionZoomOut->setObjectName(QString::fromUtf8("actionZoomOut"));
        QIcon icon16;
        icon16.addFile(QString::fromUtf8(":/images/images/ZoomOut.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionZoomOut->setIcon(icon16);
        actionZoomOut->setIconVisibleInMenu(false);
        actionZoom_Window = new QAction(MainWindow);
        actionZoom_Window->setObjectName(QString::fromUtf8("actionZoom_Window"));
        QIcon icon17;
        icon17.addFile(QString::fromUtf8(":/images/images/zoomOptions.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionZoom_Window->setIcon(icon17);
        actionPlay_Loop = new QAction(MainWindow);
        actionPlay_Loop->setObjectName(QString::fromUtf8("actionPlay_Loop"));
        QIcon icon18;
        icon18.addFile(QString::fromUtf8(":/images/images/loop.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionPlay_Loop->setIcon(icon18);
        actionValues_Cursor = new QAction(MainWindow);
        actionValues_Cursor->setObjectName(QString::fromUtf8("actionValues_Cursor"));
        actionValues_Cursor->setCheckable(true);
        QIcon icon19;
        icon19.addFile(QString::fromUtf8(":/images/images/valuesCursor.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionValues_Cursor->setIcon(icon19);
        actionMisc_Configuration = new QAction(MainWindow);
        actionMisc_Configuration->setObjectName(QString::fromUtf8("actionMisc_Configuration"));
        QIcon icon20;
        icon20.addFile(QString::fromUtf8(":/images/images/misc.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionMisc_Configuration->setIcon(icon20);
        centralWidget = new QWidget(MainWindow);
        centralWidget->setObjectName(QString::fromUtf8("centralWidget"));
        centralWidget->setStyleSheet(QString::fromUtf8(""));
        MainWindow->setCentralWidget(centralWidget);
        menuBar = new QMenuBar(MainWindow);
        menuBar->setObjectName(QString::fromUtf8("menuBar"));
        menuBar->setGeometry(QRect(0, 0, 825, 22));
        menuFile = new QMenu(menuBar);
        menuFile->setObjectName(QString::fromUtf8("menuFile"));
        menuEdit = new QMenu(menuBar);
        menuEdit->setObjectName(QString::fromUtf8("menuEdit"));
        menuView = new QMenu(menuBar);
        menuView->setObjectName(QString::fromUtf8("menuView"));
        menuWindow = new QMenu(menuView);
        menuWindow->setObjectName(QString::fromUtf8("menuWindow"));
        menuShow = new QMenu(menuView);
        menuShow->setObjectName(QString::fromUtf8("menuShow"));
        menuZoom = new QMenu(menuView);
        menuZoom->setObjectName(QString::fromUtf8("menuZoom"));
        menuType_Here = new QMenu(menuView);
        menuType_Here->setObjectName(QString::fromUtf8("menuType_Here"));
        menuTools = new QMenu(menuBar);
        menuTools->setObjectName(QString::fromUtf8("menuTools"));
        menuHelp = new QMenu(menuBar);
        menuHelp->setObjectName(QString::fromUtf8("menuHelp"));
        MainWindow->setMenuBar(menuBar);
        mainToolBar = new QToolBar(MainWindow);
        mainToolBar->setObjectName(QString::fromUtf8("mainToolBar"));
        mainToolBar->setToolTipDuration(-1);
        mainToolBar->setStyleSheet(QString::fromUtf8("\n"
"QToolButton {\n"
"background-color: lightgray;\n"
"    border-style: outset;\n"
"    border-width: 2px;\n"
"    border-radius: 10px;\n"
"    border-color: blue;\n"
"    min-width: 1em;\n"
"    padding: 1px;\n"
"}\n"
"\n"
"\n"
"QToolButton:checked{\n"
"	background-color: gray;\n"
"   border-style: inset;\n"
"}\n"
"\n"
"\n"
""));
        MainWindow->addToolBar(Qt::TopToolBarArea, mainToolBar);
        statusBar = new QStatusBar(MainWindow);
        statusBar->setObjectName(QString::fromUtf8("statusBar"));
        MainWindow->setStatusBar(statusBar);

        menuBar->addAction(menuFile->menuAction());
        menuBar->addAction(menuEdit->menuAction());
        menuBar->addAction(menuView->menuAction());
        menuBar->addAction(menuTools->menuAction());
        menuBar->addAction(menuHelp->menuAction());
        menuFile->addAction(actionNew);
        menuFile->addAction(actionOpen);
        menuFile->addAction(actionSave);
        menuFile->addAction(actionSave_Selection);
        menuFile->addAction(actionPrint);
        menuFile->addSeparator();
        menuEdit->addAction(actionUndo);
        menuEdit->addAction(actionRedo);
        menuEdit->addSeparator();
        menuEdit->addAction(actionCut);
        menuEdit->addAction(actionCopy);
        menuEdit->addAction(actionPaste);
        menuEdit->addSeparator();
        menuEdit->addAction(actionReset);
        menuEdit->addAction(actionReload);
        menuView->addAction(menuWindow->menuAction());
        menuView->addAction(menuShow->menuAction());
        menuView->addAction(menuZoom->menuAction());
        menuView->addAction(menuType_Here->menuAction());
        menuWindow->addAction(actionVsection);
        menuShow->addAction(actionMovie_Player);
        menuShow->addSeparator();
        menuShow->addAction(actionFields);
        menuShow->addAction(actionProducts);
        menuShow->addSeparator();
        menuShow->addAction(actionMaps);
        menuShow->addAction(actionData_Layers);
        menuShow->addAction(actionOverlays);
        menuShow->addAction(actionWind_Layer);
        menuShow->addAction(actionStatus_Window);
        menuShow->addAction(actionMisc_Configuration);
        menuZoom->addAction(actionZoomIn);
        menuZoom->addAction(actionZoomOut);
        menuZoom->addAction(actionZoom_Window);
        menuTools->addAction(actionPlay_Loop);
        menuTools->addAction(actionValues_Cursor);
        menuHelp->addAction(actionView_Tutorial_Page);
        mainToolBar->addAction(actionPlay_Loop);
        mainToolBar->addAction(actionUndo);
        mainToolBar->addAction(actionRedo);
        mainToolBar->addAction(actionZoomIn);
        mainToolBar->addAction(actionZoomOut);
        mainToolBar->addAction(actionZoom_Window);
        mainToolBar->addAction(actionValues_Cursor);
        mainToolBar->addAction(actionMovie_Player);
        mainToolBar->addAction(actionVsection);
        mainToolBar->addAction(actionFields);
        mainToolBar->addAction(actionProducts);
        mainToolBar->addAction(actionMaps);
        mainToolBar->addAction(actionData_Layers);
        mainToolBar->addAction(actionOverlays);
        mainToolBar->addAction(actionWind_Layer);
        mainToolBar->addAction(actionStatus_Window);
        mainToolBar->addAction(actionMisc_Configuration);

        retranslateUi(MainWindow);

        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QCoreApplication::translate("MainWindow", "LUCID Main View", nullptr));
        actionOpen->setText(QCoreApplication::translate("MainWindow", "Load Project", nullptr));
        actionSave->setText(QCoreApplication::translate("MainWindow", "Save Project", nullptr));
        actionPrint->setText(QCoreApplication::translate("MainWindow", "Print", nullptr));
        actionView_Tutorial_Page->setText(QCoreApplication::translate("MainWindow", "View CIDD Manual", nullptr));
        actionNew->setText(QCoreApplication::translate("MainWindow", "New Project", nullptr));
        actionRedo->setText(QCoreApplication::translate("MainWindow", "Redo", nullptr));
        actionCut->setText(QCoreApplication::translate("MainWindow", "Cut", nullptr));
        actionCopy->setText(QCoreApplication::translate("MainWindow", "Copy", nullptr));
        actionPaste->setText(QCoreApplication::translate("MainWindow", "Paste", nullptr));
        actionZoomIn->setText(QCoreApplication::translate("MainWindow", "Zoom In", nullptr));
#if QT_CONFIG(tooltip)
        actionZoomIn->setToolTip(QCoreApplication::translate("MainWindow", "Zoom In", nullptr));
#endif // QT_CONFIG(tooltip)
        actionVsection->setText(QCoreApplication::translate("MainWindow", "Vsection", nullptr));
        actionFields->setText(QCoreApplication::translate("MainWindow", "Fields", nullptr));
        actionMaps->setText(QCoreApplication::translate("MainWindow", "Map Layers", nullptr));
        actionProducts->setText(QCoreApplication::translate("MainWindow", "Products", nullptr));
        actionData_Layers->setText(QCoreApplication::translate("MainWindow", "Grid Data Layers", nullptr));
        actionOverlays->setText(QCoreApplication::translate("MainWindow", "Overlays", nullptr));
#if QT_CONFIG(tooltip)
        actionOverlays->setToolTip(QCoreApplication::translate("MainWindow", "Overlays", nullptr));
#endif // QT_CONFIG(tooltip)
        actionWind_Layer->setText(QCoreApplication::translate("MainWindow", "Wind Layers", nullptr));
        actionStatus_Window->setText(QCoreApplication::translate("MainWindow", "Status Window", nullptr));
        actionSave_Selection->setText(QCoreApplication::translate("MainWindow", "Save Selection", nullptr));
        actionReset->setText(QCoreApplication::translate("MainWindow", "Reset", nullptr));
        actionReload->setText(QCoreApplication::translate("MainWindow", "Reload", nullptr));
        actionMovie_Player->setText(QCoreApplication::translate("MainWindow", "Show Movie Player", nullptr));
#if QT_CONFIG(tooltip)
        actionMovie_Player->setToolTip(QCoreApplication::translate("MainWindow", "Show Movie Player Widget", nullptr));
#endif // QT_CONFIG(tooltip)
        actionUndo->setText(QCoreApplication::translate("MainWindow", "Undo", nullptr));
        actionZoomOut->setText(QCoreApplication::translate("MainWindow", "Zoom Out", nullptr));
        actionZoom_Window->setText(QCoreApplication::translate("MainWindow", "Zoom Window", nullptr));
        actionPlay_Loop->setText(QCoreApplication::translate("MainWindow", "Play Loop", nullptr));
        actionValues_Cursor->setText(QCoreApplication::translate("MainWindow", "Values Cursor", nullptr));
        actionMisc_Configuration->setText(QCoreApplication::translate("MainWindow", "Misc Configuration", nullptr));
        menuFile->setTitle(QCoreApplication::translate("MainWindow", "File", nullptr));
        menuEdit->setTitle(QCoreApplication::translate("MainWindow", "Edit", nullptr));
        menuView->setTitle(QCoreApplication::translate("MainWindow", "View", nullptr));
        menuWindow->setTitle(QCoreApplication::translate("MainWindow", "Window", nullptr));
        menuShow->setTitle(QCoreApplication::translate("MainWindow", "Show", nullptr));
        menuZoom->setTitle(QCoreApplication::translate("MainWindow", "Zoom", nullptr));
        menuType_Here->setTitle(QCoreApplication::translate("MainWindow", "Type Here", nullptr));
        menuTools->setTitle(QCoreApplication::translate("MainWindow", "Tools", nullptr));
        menuHelp->setTitle(QCoreApplication::translate("MainWindow", "Help", nullptr));
        mainToolBar->setWindowTitle(QCoreApplication::translate("MainWindow", "mainToolbar", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H

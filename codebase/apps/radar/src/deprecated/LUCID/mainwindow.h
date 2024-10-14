#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "contMainImage.h"
#include "contMiscConfig.h"
#include "contFieldDock.h"
#include "contToolBars.h"
#include "contGridConfigDialog.h"
#include "contOverlaysDialog.h"
#include "contPlayerDock.h"
#include "contStatusDialog.h"
#include "contValuesDisplay.h"
#include "contVsection.h"
#include "contWindDialog.h"
#include "contZoomOptions.h"

#include <QMainWindow>
#include <QWidget>
#include <QMouseEvent>
#include <QLabel>
#include <QAction>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    //constructor
    explicit MainWindow(QWidget *parent = nullptr);

    //destructor
    ~MainWindow();

private slots:

    //actions
    void on_actionFields_toggled(bool arg1);
    void on_actionProducts_toggled(bool arg1);
    void on_actionMaps_toggled(bool arg1);
    void on_actionMovie_Player_toggled(bool arg1);
    void on_actionZoomOut_triggered();
    void on_actionZoomIn_triggered();
    void on_actionStatus_Window_triggered();
    void on_actionVsection_triggered();
    void on_actionZoom_Window_triggered();
    void on_actionValues_Cursor_toggled(bool arg1);
    void on_actionWind_Layer_triggered();
    void on_actionData_Layers_triggered();
    void on_actionOpen_triggered();
    void on_actionMisc_Configuration_triggered();
    void on_actionOverlays_triggered();

private:
    Ui::MainWindow *ui;

    //to get the other windows going in MainWindow()
    contMainImage *mainImageController;
    contMiscConfig *miscConfigController;
    contFieldDock *fieldDockController;
    contToolBars *toolBarController;
    contGridConfigDialog *gridConfigDialogController;
    contOverlaysDialog *overlaysController;
    contPlayerDock *playerDockController;
    contStatusDialog *statusDialogController;
    contValuesDisplay *valuesDisplayController;
    contVsection *VsectionController;
    contWindDialog *windDialogController;
    contZoomOptions *zoomOptionsController;

    //for showing coordinate of cursor in main window
    void mouseMoveEvent(QMouseEvent* event);

    //spacermaker
    void toolBarSpacers();
    QWidget *emptySpacer;
};

#endif // MAINWINDOW_H









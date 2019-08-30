#ifndef MAINWINDOW_H
#define MAINWINDOW_H


#include "viewWindDialog.h"
#include "viewGridConfigDialog.h"
#include "viewVsection.h"
#include "viewStatusDialog.h"
#include "viewPlayerDock.h"
#include "viewZoomOptions.h"
#include "viewValuesDisplay.h"
#include "viewMainImage.h"

#include <QMainWindow>
#include <QToolBar>
#include <QWidget>
#include <QDockWidget>
#include <QListWidget>
#include <QVBoxLayout>
#include <QPushButton>
#include <QGridLayout>
#include <QGroupBox>
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

private:
    Ui::MainWindow *ui;

    //for showing coordinate of cursor in main window
    void mouseMoveEvent(QMouseEvent* event);

    //to get the other windows going in MainWindow()
    viewVsection *Vsec;
    viewStatusDialog *Stat;
    viewWindDialog *windDialog;
    viewGridConfigDialog *gridConfigDialog;
    viewPlayerDock *playerDock;
    viewZoomOptions *zoomWindow;
    viewValuesDisplay *valuesWindow;
    viewMainImage *centralImage;

    //toolbar maker
    void startToolBars();
    QToolBar *products, *maps;

    //spacermaker
    void toolBarSpacers();
    QWidget *emptySpacer;

    //field dock maker
    void fieldDockMaker();
    QDockWidget *fieldDock;
    QListWidget *fieldList;
    QVBoxLayout *fieldLayout;
    QPushButton *field[3][10];
    QGridLayout *grid;
    QGroupBox *fieldGroup;

};

#endif // MAINWINDOW_H

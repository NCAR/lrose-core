#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QFileDialog>
#include <iostream>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    MainWindow::move(100,0);
    MainWindow::toolBarSpacers(); //create spacer for the main toobar

    mainImageController = new contMainImage(R"(:/images/images/example.png)", this);
    setCentralWidget(mainImageController->viewer);

    fieldDockController = new contFieldDock;
    MainWindow::addDockWidget(Qt::RightDockWidgetArea, fieldDockController->fieldDock);
    fieldDockController->fieldDock->hide();

    toolBarController = new contToolBars;
    this->addToolBar(Qt::RightToolBarArea, toolBarController->products);
    this->addToolBar(Qt::RightToolBarArea, toolBarController->maps);
    toolBarController->products->hide();
    toolBarController->maps->hide();

    gridConfigDialogController = new contGridConfigDialog;

    overlaysController = new contOverlaysDialog;
    overlaysController->overlaysViewer->hide();

    playerDockController = new contPlayerDock;
    MainWindow::addDockWidget(Qt::BottomDockWidgetArea, playerDockController->playerDockViewer);
    playerDockController->playerDockViewer->hide();

    statusDialogController = new contStatusDialog;
    statusDialogController->statusDialogViewer->hide();

    valuesDisplayController = new contValuesDisplay;
    valuesDisplayController->valuesDisplayViewer->hide();

    VsectionController = new contVsection;
    VsectionController->VsectionViewer->hide();

    windDialogController = new contWindDialog;
    windDialogController->windDialogViewer->hide();

    zoomOptionsController = new contZoomOptions;
    zoomOptionsController->zoomOptionsViewer->hide();

    miscConfigController = new contMiscConfig;
    miscConfigController->miscConfigViewer->hide();


    //just a tester to access function in other file/class
    //totally delete this later on
//    viewMainImage MMM("wut");
//    MMM.Tester();

    this->setMouseTracking(0);
}

MainWindow::~MainWindow()
{
    delete ui;
}


//-------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------
//functions and whatnot



//this little function make it show the coordinates of the cursor as a tool tip
//it is activated by "void MainWindow::on_actionValues_Cursor_toggled(bool arg1)"
void MainWindow::mouseMoveEvent(QMouseEvent* event)
{
    QPoint p = mainImageController->viewer->scrollArea->mapFromGlobal(event->globalPos());
    valuesDisplayController->updateValues(event, p);
}

//make spacers to go in the main toolbar at the top of the main window.
//also ready to add spacers elsewhere
void MainWindow::toolBarSpacers()
{
    QWidget *emptySpacer = new QWidget();
    //QWidget *empty2 = new QWidget();
    emptySpacer->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Preferred);
    //empty2->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Preferred);
    ui->mainToolBar->insertWidget(ui->actionMovie_Player, emptySpacer);
    //ui->mainToolBar->insertWidget(ui->actionVsection, empty2);
}



//-------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------
//SLOTS n actions n stuff



void MainWindow::on_actionFields_toggled(bool arg1)
{
    if (arg1==0){
        fieldDockController->fieldDock->hide();
    }
    else {
        fieldDockController->fieldDock->show();
    }
}

void MainWindow::on_actionProducts_toggled(bool arg1)
{
    if (arg1==0){
        toolBarController->products->hide();
    }
    else {
        toolBarController->products->show();
    }
}

void MainWindow::on_actionMaps_toggled(bool arg1)
{
    if (arg1==0){
        toolBarController->maps->hide();
    }
    else {
        toolBarController->maps->show();
    }
}

void MainWindow::on_actionMovie_Player_toggled(bool arg1)
{

    if (arg1==0){
        playerDockController->playerDockViewer->hide();
    }
    else {
        playerDockController->playerDockViewer->show();
    }
}





void MainWindow::on_actionStatus_Window_triggered()
{
    //this reads the current position and size of the main window
    //then places the status window to the right of it
    //shows it,
    //and raises it in case it got covered up.
    QPoint p = MainWindow::pos();
    QSize  s = MainWindow::frameSize();
    statusDialogController->statusDialogViewer->move(p.x()+s.width(), p.y()+75);
    statusDialogController->statusDialogViewer->show();
    statusDialogController->statusDialogViewer->raise();
}

void MainWindow::on_actionVsection_triggered()
{
    //this reads the current position and size of the main window
    //then places the Vsection window to the right of it
    //shows it,
    //and raises it in case it got covered up.
    QPoint p = MainWindow::pos();
    QSize  s = MainWindow::frameSize();
    VsectionController->VsectionViewer->move(p.x()+s.width(), p.y());
    VsectionController->VsectionViewer->show();
    VsectionController->VsectionViewer->raise();
}

void MainWindow::on_actionZoom_Window_triggered()
{
    //this reads the current position and size of the main window
    //then places the zooms window to the right of it
    //shows it,
    //and raises it in case it got covered up.
    QPoint p = MainWindow::pos();
    QSize  s = MainWindow::frameSize();
    zoomOptionsController->zoomOptionsViewer->move(p.x()+s.width(), p.y()+100);
    zoomOptionsController->zoomOptionsViewer->show();
    zoomOptionsController->zoomOptionsViewer->raise();
}

void MainWindow::on_actionValues_Cursor_toggled(bool arg1)
{
    if (arg1==0){
        valuesDisplayController->valuesDisplayViewer->hide();
        this->setMouseTracking(false);
    }
    else {
        //this reads the current position and size of the main window
        //then places the values window to the right of it
        //shows it,
        //and raises it in case it got covered up.
        QPoint p = MainWindow::pos();
        QSize  s = MainWindow::frameSize();
        valuesDisplayController->valuesDisplayViewer->move(p.x()+s.width(), p.y()+125);
        valuesDisplayController->valuesDisplayViewer->show();
        valuesDisplayController->valuesDisplayViewer->raise();
        this->setMouseTracking(true);
    }
}

void MainWindow::on_actionWind_Layer_triggered()
{
    //this reads the current position and size of the main window
    //then places the wind configuration window to the right of it
    //shows it,
    //and raises it in case it got covered up.
    QPoint p = MainWindow::pos();
    QSize  s = MainWindow::frameSize();
    windDialogController->windDialogViewer->move(p.x()+s.width(), p.y()+50);
    windDialogController->windDialogViewer->show();
    windDialogController->windDialogViewer->raise();
}

void MainWindow::on_actionData_Layers_triggered()
{
    //this reads the current position and size of the main window
    //then places the grid configuration window to the right of it
    //shows it,
    //and raises it in case it got covered up.
    QPoint p = MainWindow::pos();
    QSize  s = MainWindow::frameSize();
    gridConfigDialogController->gridConfigViewer->move(p.x()+s.width(), p.y()+25);
    gridConfigDialogController->gridConfigViewer->show();
    gridConfigDialogController->gridConfigViewer->raise();

}

void MainWindow::on_actionOpen_triggered()
{
    //right now, the open file option really is just good for setting a new image in the main window
    //this should get MUCH more complicated.
    //open a file dialog to get new image address
    QString fileName = QFileDialog::getOpenFileName(this,
                                                    "Open the file",
                                                    "/Users",
                                                    "Images (*.png *.xpm *.jpg)"); //opens a dialog box "open..." titles the dia box
    if(!fileName.isEmpty())
    {
        //send to viewmainimage to set up a new main image.
        //there is no filter yet, so I don't know what all can happen if a
        //file is selected that is not an image file.
        contMainImage *temp = new contMainImage(fileName, this);
        setCentralWidget(temp);
    }
}

void MainWindow::on_actionMisc_Configuration_triggered()
{
    //this reads the current position and size of the main window
    //then places the miscellanous configuration window to the right of it
    //shows it,
    //and raises it in case it got covered up.
    QPoint p = MainWindow::pos();
    QSize  s = MainWindow::frameSize();
    miscConfigController->miscConfigViewer->move(p.x()+s.width(), p.y()+100);
    miscConfigController->miscConfigViewer->show();
    miscConfigController->miscConfigViewer->raise();
}

void MainWindow::on_actionOverlays_triggered()
{
    QPoint p = MainWindow::pos();
    QSize  s = MainWindow::frameSize();
    overlaysController->overlaysViewer->move(p.x()+s.width(), p.y()+100);
    overlaysController->overlaysViewer->show();
    overlaysController->overlaysViewer->raise();
}




//cruddy zooms that need replacing
void MainWindow::on_actionZoomOut_triggered()
{

}

void MainWindow::on_actionZoomIn_triggered()
{

}













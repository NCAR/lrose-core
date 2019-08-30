#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QFileDialog>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    MainWindow::move(100,0);

    //MainWindow::startImage(); //create main dummy image and scale bar
    MainWindow::startToolBars(); //create dummy toolbars for products, and maps
    MainWindow::toolBarSpacers(); //create spacer for the main toobar
    MainWindow::fieldDockMaker(); //create dummy field dock

    Vsec = new viewVsection(this);
    //V_section(this) the 'this' makes the main window the parent, so if the parent is closed, then the window is closed too.
    Stat = new viewStatusDialog(this);
    windDialog = new viewWindDialog(this);
    gridConfigDialog = new viewGridConfigDialog(this);
    zoomWindow = new viewZoomOptions(this);
    valuesWindow = new viewValuesDisplay(this);

    playerDock = new viewPlayerDock(this);
    MainWindow::addDockWidget(Qt::BottomDockWidgetArea, playerDock);
    playerDock->hide();

    centralImage = new viewMainImage(R"(:/images/images/example.png)", this);
    setCentralWidget(centralImage);

    //just a tester to access function in other file/class
    //totally delete this later on
    viewMainImage MMM("wut");
    MMM.Tester();

}

MainWindow::~MainWindow()
{
    delete ui;
}


//-------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------
//functions and whatnot


//this little function make it so that when you hold left click in the main window,
//it shows the coordinates of the cursor
void MainWindow::mouseMoveEvent(QMouseEvent* event)
{
    valuesWindow->valueOf1->setText(QString::number( event->pos().x() ));
    valuesWindow->valueOf2->setText(QString::number( event->pos().y() ));
    valuesWindow->valueOf3->setText(QString::number( event->pos().x() ));
    valuesWindow->valueOf4->setText(QString::number( event->pos().y() ));
    QToolTip::showText(event->globalPos(),
                       //  In most scenarios you will have to change these for
                       //  the coordinate system you are working in.
                       QString::number( event->pos().x() ) + ", " +
                       QString::number( event->pos().y() ));
}



//create dummy toolbars for products, and maps
void MainWindow::startToolBars()
{
    //products toolbar
    products = this->addToolBar(tr("productsToolbar"));
    addToolBar(Qt::RightToolBarArea, products);
    QString allProducts = "All Products";
    products->addAction(allProducts);
    for (int i=1; i<30; i++)
    {
        QString nameOfAction = "Product: " + QString::number(i);
        products ->addAction(nameOfAction);
    }
    products->hide();


    //maps toolbar
    maps = this->addToolBar(tr("mapOverlaysToobar"));
    addToolBar(Qt::RightToolBarArea, maps);
    QString allMaps = "All Maps";
    maps->addAction(allMaps);
    for (int i=1; i<30; i++)
    {
        QString nameOfAction = "Map: " + QString::number(i);
        maps->addAction(nameOfAction);
    }
    maps->hide();
}


//make spacers to go in the main toolbar at the top of the main window.
//also ready to add spacers elsewhere
void MainWindow::toolBarSpacers()
{
    QWidget *emptySpacer = new QWidget();
    //QWidget *empty2 = new QWidget();
    //QWidget *empty3 = new QWidget();
    emptySpacer->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Preferred);
    //empty2->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Preferred);
    //empty3->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Preferred);
    ui->mainToolBar->insertWidget(ui->actionMovie_Player, emptySpacer);
    //ui->mainToolBar->insertWidget(ui->actionVsection, empty2);
    //ui->mainToolBar->insertWidget(ui->actionVsection, empty3);
}


//create dummy field dock
void MainWindow::fieldDockMaker()
{
    //initialize field dock and specify properties
    fieldDock = new QDockWidget;
    QString fieldTitle = "Field Options";
    fieldDock->setWindowTitle(fieldTitle);
    fieldDock->setAllowedAreas(Qt::DockWidgetAreas(Qt::RightDockWidgetArea |
                                                   Qt::LeftDockWidgetArea));
    fieldDock->setFeatures(QDockWidget::DockWidgetFeatures(QDockWidget::DockWidgetClosable |
                                                           QDockWidget::DockWidgetMovable |
                                                           QDockWidget::DockWidgetFloatable));


    //create list widget as a dummy for that list thing in the fields window
    fieldList = new QListWidget(this);
    for(int i=0; i<10; i++)
    {
        fieldList->addItems(QStringList() << "Field Option");
    }


    //make grid of pushbuttons.
    //not sure how to declare a variable amount of pushbuttons, because of memory allocation
    //SO, current plan is to declare way too many buttons, then only show the ones that
    //actually get used.
    //WILL NEED A "FIELD COUNTER" TO MAKE LIMITS FOR THE LOOPS ON HOW MANY BUTTONS TO DISPLAY
    int count = 0;
    for(int i=0; i<3; i++)
    {
        for(int j=0; j<10; j++)
        {
            count++;
            field[i][j] = new QPushButton(tr("field ")+QString::number(count));
        }
    }
    grid = new QGridLayout;
    for(int i=0; i<2; i++)
    {
        for(int j=0; j<10; j++)
        {
            grid->addWidget(field[i][j],j,i);
        }
    }


    //add list, and buttons to layout
    fieldLayout = new QVBoxLayout();
    fieldLayout->addWidget(fieldList);
    fieldLayout->addLayout(grid);

    //make grid and add layout to it to add to the widget
    fieldGroup = new QGroupBox;
    fieldGroup->setLayout(fieldLayout);
    fieldDock->setWidget(fieldGroup);

    MainWindow::addDockWidget(Qt::RightDockWidgetArea, fieldDock);
    fieldDock->hide();
}





//-------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------
//SLOTS n actions n stuff






void MainWindow::on_actionFields_toggled(bool arg1)
{
    if (arg1==0){
        fieldDock->hide();
    }
    else {
        fieldDock->show();
    }
}

void MainWindow::on_actionProducts_toggled(bool arg1)
{
    if (arg1==0){
        products->hide();
    }
    else {
        products->show();
    }
}

void MainWindow::on_actionMaps_toggled(bool arg1)
{
    if (arg1==0){
        maps->hide();
    }
    else {
        maps->show();
    }
}

void MainWindow::on_actionMovie_Player_toggled(bool arg1)
{

    if (arg1==0){
        playerDock->hide();
    }
    else {
        playerDock->show();
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
    Stat->move(p.x()+s.width(), p.y()+75);
    Stat->show();
    Stat->raise();
}

void MainWindow::on_actionVsection_triggered()
{
    //this reads the current position and size of the main window
    //then places the Vsection window to the right of it
    //shows it,
    //and raises it in case it got covered up.
    QPoint p = MainWindow::pos();
    QSize  s = MainWindow::frameSize();
    Vsec->move(p.x()+s.width(), p.y());
    Vsec->show();
    Vsec->raise();
}

void MainWindow::on_actionZoom_Window_triggered()
{
    //this reads the current position and size of the main window
    //then places the zooms window to the right of it
    //shows it,
    //and raises it in case it got covered up.
    QPoint p = MainWindow::pos();
    QSize  s = MainWindow::frameSize();
    zoomWindow->move(p.x()+s.width(), p.y()+100);
    zoomWindow->show();
    zoomWindow->raise();
}

void MainWindow::on_actionValues_Cursor_toggled(bool arg1)
{
    if (arg1==0){
        valuesWindow->hide();
        this->setMouseTracking(false);
    }
    else {
        //this reads the current position and size of the main window
        //then places the values window to the right of it
        //shows it,
        //and raises it in case it got covered up.
        QPoint p = MainWindow::pos();
        QSize  s = MainWindow::frameSize();
        valuesWindow->move(p.x()+s.width(), p.y()+125);
        valuesWindow->show();
        valuesWindow->raise();
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
    windDialog->move(p.x()+s.width(), p.y()+50);
    windDialog->show();
    windDialog->raise();
}

void MainWindow::on_actionData_Layers_triggered()
{
    //this reads the current position and size of the main window
    //then places the grid configuration window to the right of it
    //shows it,
    //and raises it in case it got covered up.
    QPoint p = MainWindow::pos();
    QSize  s = MainWindow::frameSize();
    gridConfigDialog->move(p.x()+s.width(), p.y()+25);
    gridConfigDialog->show();
    gridConfigDialog->raise();
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
        viewMainImage *temp = new viewMainImage(fileName, this);
        setCentralWidget(temp);
    }
}


//cruddy zooms that need replacing
void MainWindow::on_actionZoomOut_triggered()
{

}

void MainWindow::on_actionZoomIn_triggered()
{

}





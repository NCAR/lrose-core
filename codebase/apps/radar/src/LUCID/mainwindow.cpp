#include "mainwindow.h"
#include "ui_mainwindow.h"


#include <QLocale>
#include <QDateTime>
//#include <Q




MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    //------
    //this starts a timer that loops every second, it is currently being used for the clock on the Main Window
    //The timer can be used to for other things as well though.
    timer_1s = new QTimer(this);
    QObject::connect(timer_1s, SIGNAL(timeout()), this, SLOT(UpdateTime()));
    timer_1s->start(1000);
    //------

    MainWindow::startImage(); //create main dummy image and scale bar
    MainWindow::startToolBars(); //create dummy toolbars for fields, products, and maps
    MainWindow::toolBarSpacers(); //create spacer for the main toobar
    MainWindow::fieldDockMaker(); //create dummy field dock
    MainWindow::movieLooperDock(); //create movie player dock


    Vsec = new V_section(this); //this line makes a new Vsection box
    V_section(this); // the 'this' makes the main window the parent, so if the parent is closed, then the window is closed too.

    Stat = new StatusDialog(this);
    StatusDialog(this);

    connect(ui->actionQuit_CIDD, SIGNAL(triggered()), this, SLOT(close()));


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
    QToolTip::showText(event->globalPos(),
                       //  In most scenarios you will have to change these for
                       //  the coordinate system you are working in.
                       QString::number( event->pos().x() ) + ", " +
                       QString::number( event->pos().y() ));
}



//create main dummy image and scale bar
void MainWindow::startImage()
{
    //initialize vars and ptrs
    QString pic = R"(/Users/katsampe/Desktop/example.png)";
    //mainImage = new QLabel;
    //img = new QPixmap(pic);
    QImage image(pic);
    item = new QGraphicsPixmapItem(QPixmap::fromImage(image));
    scene = new QGraphicsScene;
    scene->addItem(item);
    view = new QGraphicsView(scene);
    view->setMouseTracking(true);



    QString scalePic = R"(/Users/katsampe/Desktop/dBzScale.png)";
    scale = new QLabel;
    QPixmap *scaleImg = new QPixmap(scalePic);
    currentTime = new QLabel;


    //mod main image
    //mainImage->setPixmap(*img);
    //mainImage->setMinimumSize(100, 100);
    //mainImage->setScaledContents(1);

    //mod scale image
    scale->setPixmap(*scaleImg);
    scale->setScaledContents(1);
    scale->setMinimumSize(35, 1);
    scale->setMaximumSize(36, 10000);

    //add to layouts
    mainLayout = new QHBoxLayout;
    //mainLayout->addWidget(mainImage);
    mainLayout->addWidget(view);
    mainLayout->addWidget(scale);
    QWidget *w = new QWidget;
    w->setLayout(mainLayout);



    timeLayout = new QVBoxLayout;
    timeLayout->addWidget(currentTime);
    timeLayout->addWidget(w);


    //add to MainWindow
    QWidget *placeholderWidget = new QWidget;
    placeholderWidget->setLayout(timeLayout);

    setCentralWidget(placeholderWidget);
    //this->setCentralWidget(mainImage);
    centralWidget()->setMouseTracking(true);


}




//create dummy toolbars for fields, products, and maps
void MainWindow::startToolBars()
{
    //fields toolbar
    fields = this->addToolBar(tr("fieldsToolbar"));
    addToolBar(Qt::LeftToolBarArea, fields);
    QString allFields = "All Fields";
    fields->addAction(allFields);
    for (int i=1; i<10; i++)
    {
        QString nameOfAction = "Field: " + QString::number(i);
        fields->addAction(nameOfAction);
    }
    fields->hide();


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
        fieldList->addItems(QStringList() << "field name I think?");
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




//create movie player dock
void MainWindow::movieLooperDock()
{
    //initialize movie dock, and set properties
    movieDock = new QDockWidget;
    QString movieTitle = "Movie Player";
    movieDock->setWindowTitle(movieTitle);
    movieDock->setAllowedAreas(Qt::DockWidgetAreas(Qt::TopDockWidgetArea |
                                                   Qt::BottomDockWidgetArea));
    movieDock->setFeatures(QDockWidget::DockWidgetFeatures(QDockWidget::DockWidgetClosable |
                                                           QDockWidget::DockWidgetMovable |
                                                           QDockWidget::DockWidgetFloatable));


    //make widgets for movie player
    //TOP ROW------------------------------------------
    //frame label/indicator
    frameLabel = new QLabel;
    QString frameLabelText = "Frame";
    frameLabel->setText(frameLabelText);
    frameIndicator = new QLCDNumber;

    //buttons
    rwd = new QToolButton;
    rwd->setStyleSheet(QString("QToolButton{ background-color:lightgray;}"));
    rwd->setIcon(QIcon(QString(":/images/rwd.png")));
    play = new QToolButton;
    play->setStyleSheet(QString("QToolButton{ background-color:lightgray;}"));
    play->setIcon(QIcon(QString(":/images/play.png")));
    pause = new QToolButton;
    pause->setStyleSheet(QString("QToolButton{ background-color:lightgray;}"));
    pause->setIcon(QIcon(QString(":/images/pause.png")));
    fwd = new QToolButton;
    fwd->setStyleSheet(QString("QToolButton{ background-color:lightgray;}"));
    fwd->setIcon(QIcon(QString(":/images/ffwd.png")));

    //slider
    posIndicator = new QSlider(Qt::Horizontal);
    posIndicator->setTickPosition(QSlider::TicksBelow);
    posIndicator->setTickInterval(1);

    //time and date display
    frameTime = new QLabel;
    frameTime->setText("12:15:33");
    frameDate = new QLabel;
    frameDate->setText("08/15/2019");

    //MIDDLE ROW--------------------------------------
    //time label/input
    timeLabel = new QLabel;
    QString timeLabelText = "Start Time & Date";
    timeLabel->setText(timeLabelText);
    timeInput = new QDateTimeEdit;

    //frame interval label/input
    frameIntervalLabel = new QLabel;
    QString frameIntervalLabelText = "Frame Times(min)";
    frameIntervalLabel->setText(frameIntervalLabelText);
    frameIntervalInput = new QLineEdit;

    //number of frames label/input
    numFramesLabel = new QLabel;
    QString numFramesLabelText = "Number of Frames";
    numFramesLabel->setText(numFramesLabelText);
    numFramesInput = new QLineEdit;
    numFramesInput->insert("100");

    //BOTTOM ROW-------------------------------------
    //label for playback speed
    playbackLabel = new QLabel;
    playbackLabel->setText("Playback Speed");

    //combobox for playback speed
    playback = new QComboBox;
    playback->addItem("1 frame per second?");
    playback->addItem("5 frames per second?");
    playback->addItem("10 frames per second?");

    //label for delay
    delayLabel = new QLabel;
    delayLabel->setText("Delay Between Loops");

    //combobox for delay
    delay = new QComboBox;
    delay->addItem("1 second");
    delay->addItem("5 seconds");
    delay->addItem("10 seconds");

    //combobox for realtime/archive time
    realArchive = new QComboBox;
    realArchive->addItem("Archive Time");
    realArchive->addItem("Real Time");

    //combobox for loop/sweep/no repeat
    loopSweep = new QComboBox;
    loopSweep->addItem("No Repeat");
    loopSweep->addItem("Loop");
    loopSweep->addItem("Sweep");



    //intialize layouts
    topRow = new QHBoxLayout();
    midRow = new QHBoxLayout();
    botRow = new QHBoxLayout();
    threeRows = new QVBoxLayout();
    group = new QGroupBox;


    //add widgets to layouts (in order), with spcaing for where things are too close to eachother
    topRow->addWidget(frameLabel);
    topRow->addWidget(frameIndicator);
         topRow->addSpacing(10);
    topRow->addWidget(rwd);
    topRow->addWidget(play);
    topRow->addWidget(pause);
    topRow->addWidget(fwd);
    topRow->addWidget(posIndicator);
    topRow->addWidget(frameTime);
    topRow->addWidget(frameDate);

    midRow->addWidget(timeLabel);
    midRow->addWidget(timeInput);
        midRow->addSpacing(10);
    midRow->addWidget(frameIntervalLabel);
    midRow->addWidget(frameIntervalInput);
        midRow->addSpacing(10);
    midRow->addWidget(numFramesLabel);
    midRow->addWidget(numFramesInput);

    botRow->addWidget(playbackLabel);
    botRow->addWidget(playback);
        botRow->addSpacing(10);
    botRow->addWidget(delayLabel);
    botRow->addWidget(delay);
        botRow->addSpacing(10);
    botRow->addWidget(realArchive);
        botRow->addSpacing(10);
    botRow->addWidget(loopSweep);


    //combining the 3 rows
    threeRows->setSpacing(3);
    threeRows->addLayout(topRow);
    threeRows->addLayout(midRow);
    threeRows->addLayout(botRow);

    //adding layout to the dock widget
    group->setLayout(threeRows);
    movieDock->setWidget(group);

    //these two connects are for the slider and the number of frames, it was a toughie had to make function frameChanged()
    connect(posIndicator, SIGNAL(sliderReleased()), this, SLOT(frameChanged()));
    connect(numFramesInput, SIGNAL(returnPressed()), this, SLOT(frameChanged()));

    //add dock to main window
    MainWindow::addDockWidget(Qt::BottomDockWidgetArea, movieDock);
    movieDock->hide();
}




//-------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------
//SLOTS n actions n stuff


void MainWindow::UpdateTime()
{
    QDateTime local(QDateTime::currentDateTime());
    QDateTime UTC(local.toUTC());
    QString temp = QLocale("en_EN").toString(UTC, "dddd MMMM dd yyyy  hh:mm:ss (UTC)");
    currentTime->setText(temp);
}




//function is used in moviedock,
//it's for connectiong the slider, frame indicator, and number of frames input.
void MainWindow::frameChanged()
{
    QString num = numFramesInput->text();
    float numF = num.toFloat();
    float pos = posIndicator->sliderPosition();
    int frame = int(ceil(pos*(numF/100)));
    frameIndicator->display(frame);
}

void MainWindow::on_actionFields_toggled(bool arg1)
{
    if (arg1==0){
        fields->hide();
        fieldDock->hide();
    }
    else {
        fields->show();
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
        movieDock->hide();
    }
    else {
        movieDock->show();
    }
}

void MainWindow::on_actionStatus_Window_triggered()
{
    Stat->show();
    Stat->raise();
}

void MainWindow::on_actionVsection_triggered()
{
    Vsec->show();
    Vsec->raise();
}



//cruddy zooms that need replacing
void MainWindow::on_actionZoomOut_triggered()
{
    *img = img->scaled(img->width()-100,img->height()-100); //image scaling
    mainImage->setPixmap(*img);
}

void MainWindow::on_actionZoomIn_triggered()
{
    *img = img->scaled(img->width()+100,img->height()+100); //image scaling
    mainImage->setPixmap(*img);
}


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
    MainWindow::move(100,0);
    //------
    //this starts a timer that loops every second, it is currently being used for the clock on the Main Window
    //The timer can be used to for other things as well though.
    timer_1s = new QTimer(this);
    QObject::connect(timer_1s, SIGNAL(timeout()), this, SLOT(UpdateTime()));
    timer_1s->start(1000);
    //------

    MainWindow::startImage(); //create main dummy image and scale bar
    MainWindow::startToolBars(); //create dummy toolbars for products, and maps
    MainWindow::toolBarSpacers(); //create spacer for the main toobar
    MainWindow::fieldDockMaker(); //create dummy field dock
    MainWindow::movieLooperDock(); //create movie player dock
    MainWindow::makeZoomOptions();
    MainWindow::makeValuesDisplay();
    MainWindow::makeWindsConfiguration();
    MainWindow::makeGridConfiguration();


    Vsec = new V_section(this); //this line makes a new Vsection box
    V_section(this); // the 'this' makes the main window the parent, so if the parent is closed, then the window is closed too.

    Stat = new StatusDialog(this);
    StatusDialog(this);

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
    QString pic = R"(:/images/images/example.png)";
    QImage image(pic);
    item = new QGraphicsPixmapItem(QPixmap::fromImage(image));
    scene = new QGraphicsScene;
    scene->addItem(item);
    view = new QGraphicsView(scene);
    view->setMouseTracking(true);



    QString scalePic = R"(:/images/images/dBzScale.png)";  
    scale = new QLabel;
    QPixmap *scaleImg = new QPixmap(scalePic);

    currentTime = new QLabel;


    //mod scale image
    scale->setPixmap(*scaleImg);
    scale->setScaledContents(1);
    scale->setMinimumSize(35, 1);
    scale->setMaximumSize(36, 10000);

    //add to layouts
    mainLayout = new QHBoxLayout;
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
    rwd->setIcon(QIcon(QString(":/images/images/rwd.png")));
    play = new QToolButton;
    play->setStyleSheet(QString("QToolButton{ background-color:lightgray;}"));
    play->setIcon(QIcon(QString(":/images/images/play.png")));
    pause = new QToolButton;
    pause->setStyleSheet(QString("QToolButton{ background-color:lightgray;}"));
    pause->setIcon(QIcon(QString(":/images/images/pause.png")));
    fwd = new QToolButton;
    fwd->setStyleSheet(QString("QToolButton{ background-color:lightgray;}"));
    fwd->setIcon(QIcon(QString(":/images/images/ffwd.png")));

    //slider
    posIndicator = new QSlider(Qt::Horizontal);
    posIndicator->setTickPosition(QSlider::TicksBelow);
    posIndicator->setTickInterval(1);
    posIndicator->setMinimumWidth(400);

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
    frameIntervalInput->setMaximumWidth(100);

    //number of frames label/input
    numFramesLabel = new QLabel;
    QString numFramesLabelText = "Number of Frames";
    numFramesLabel->setText(numFramesLabelText);
    numFramesInput = new QLineEdit;
    numFramesInput->insert("100");
    numFramesInput->setMaximumWidth(100);

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

    QString sliderImageText = R"(:/images/images/movieImage.png)";
    QImage sliderImage(sliderImageText);
    sliderItem = new QGraphicsPixmapItem(QPixmap::fromImage(sliderImage));
    sliderScene = new QGraphicsScene;
    sliderScene->addItem(sliderItem);
    sliderView = new QGraphicsView(sliderScene);



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
        topRow->addStretch(0);

    midRow->addWidget(timeLabel);
    midRow->addWidget(timeInput);
        midRow->addSpacing(10);
    midRow->addWidget(frameIntervalLabel);
    midRow->addWidget(frameIntervalInput);
        midRow->addSpacing(10);
    midRow->addWidget(numFramesLabel);
    midRow->addWidget(numFramesInput);
        midRow->addStretch(0);

    botRow->addWidget(playbackLabel);
    botRow->addWidget(playback);
        botRow->addSpacing(10);
    botRow->addWidget(delayLabel);
    botRow->addWidget(delay);
        botRow->addSpacing(10);
    botRow->addWidget(realArchive);
        botRow->addSpacing(10);
    botRow->addWidget(loopSweep);
        botRow->addStretch(0);


    //combining the 3 rows
    threeRows->setSpacing(3);
    threeRows->addLayout(topRow);
    threeRows->addLayout(midRow);
    threeRows->addLayout(botRow);
    threeRows->addWidget(sliderView);



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




void MainWindow::makeZoomOptions()
{
    zoomOptions = new QDialog(this);
    zoomOptions->setWindowTitle("Zoomz");
    zoom10 = new QPushButton;
    zoom10->setText("Zoom 10");
    zoom100 = new QPushButton;
    zoom100->setText("Zoom 100");
    zoom1000 = new QPushButton;
    zoom1000->setText("Zoom 1000");
    zoomSaved = new QPushButton;
    zoomSaved->setText("Saved Zoom 1");
    zoomSaved2 = new QPushButton;
    zoomSaved2->setText("Saved Zoom 2");
    zoomReset = new QPushButton;
    zoomReset->setText("Reset Zoom");

    zoomLayout = new QVBoxLayout;
    zoomLayout->addWidget(zoomReset);
    zoomLayout->addWidget(zoom10);
    zoomLayout->addWidget(zoom100);
    zoomLayout->addWidget(zoom1000);
    zoomLayout->addWidget(zoomSaved);
    zoomLayout->addWidget(zoomSaved2);

    zoomOptions->setLayout(zoomLayout);
}


void MainWindow::makeValuesDisplay()
{
    valuesDisplay = new QDialog(this);
    valuesDisplay->setWindowTitle("Values");
    valueLabel1 = new QLabel;
    valueLabel1->setText("Value of 1: ");
    valueLabel2 = new QLabel;
    valueLabel2->setText("Value of 2: ");
    valueLabel3 = new QLabel;
    valueLabel3->setText("Value of 3: ");
    valueLabel4 = new QLabel;
    valueLabel4->setText("Value of 4: ");

    valueOf1 = new QTextBrowser;
    valueOf1->setText("0.01");
    valueOf1->setMaximumHeight(26);
    valueOf2 = new QTextBrowser;
    valueOf2->setText("0.02");
    valueOf2->setMaximumHeight(26);
    valueOf3 = new QTextBrowser;
    valueOf3->setText("0.03");
    valueOf3->setMaximumHeight(26);
    valueOf4 = new QTextBrowser;
    valueOf4->setText("0.04");
    valueOf4->setMaximumHeight(26);

    valueCombo1 = new QHBoxLayout;
    valueCombo1->addWidget(valueLabel1);
    valueCombo1->addWidget(valueOf1);
    valueCombo2 = new QHBoxLayout;
    valueCombo2->addWidget(valueLabel2);
    valueCombo2->addWidget(valueOf2);
    valueCombo3 = new QHBoxLayout;
    valueCombo3->addWidget(valueLabel3);
    valueCombo3->addWidget(valueOf3);
    valueCombo4 = new QHBoxLayout;
    valueCombo4->addWidget(valueLabel4);
    valueCombo4->addWidget(valueOf4);

    valuesLayout = new QVBoxLayout;
    valuesLayout->addLayout(valueCombo1);
    valuesLayout->addLayout(valueCombo2);
    valuesLayout->addLayout(valueCombo3);
    valuesLayout->addLayout(valueCombo4);

    valuesDisplay->setLayout(valuesLayout);
}





void MainWindow::makeWindsConfiguration()
{
    windsConfig = new QDialog(this);
    windsConfig->setWindowTitle("Winds Configuration");

    windSelectLabel = new QLabel;
    windSelectLabel->setText("Select Wind Layer:");
    windSelector = new QComboBox;
    windSelector->addItem("Surf");
    windSelector->addItem("RUC");
    windSelector->addItem("VDRAS");
    windSelector->addItem("DDOP");

    windUrlLabel = new QLabel;
    windUrlLabel->setText("Url:");
    windUrlInput = new QLineEdit;

    windColorSelectText = new QLabel;
    windColorSelectText->setText("Select Color:");
    windColorSelect = new QComboBox;
    windColorSelect->addItem("yellow");
    windColorSelect->addItem("cyan");
    windColorSelect->addItem("orange");
    windColorSelect->addItem("red");

    windNumLabel = new QLabel;
    windNumLabel->setText("Number:");
    windNumSlider = new QSlider(Qt::Horizontal, windsConfig);

    windWidthLabel = new QLabel;
    windWidthLabel->setText("Width:");
    windWidthSlider = new QSlider(Qt::Horizontal, windsConfig);

    windLengthLabel = new QLabel;
    windLengthLabel->setText("Length:");
    windLengthSlider = new QSlider(Qt::Horizontal, windsConfig);

    UNameLabel = new QLabel;
    UNameLabel->setText("UName:");
    UNameInput = new QLineEdit;
    UNameInput->setText("UWind");

    VNameLabel = new QLabel;
    VNameLabel->setText("VName:");
    VNameInput = new QLineEdit;
    VNameInput->setText("VWind");

    WNameLabel = new QLabel;
    WNameLabel->setText("WName:");
    WNameInput = new QLineEdit;
    WNameInput->setText("NA");

    windTimeSlopLabel = new QLabel;
    windTimeSlopLabel->setText("Time Slop:");
    windTimeSlopInput = new QLineEdit;
    windTimeSlopInput->setText("100v/t");

    windTimeOffsetLabel = new QLabel;
    windTimeOffsetLabel->setText("Time Offset:");
    windTimeOffsetInput = new QLineEdit;
    windTimeOffsetInput->setText("0v/t");

    windAltitudeOffsetLabel = new QLabel;
    windAltitudeOffsetLabel->setText("Altitude Offset:");
    windAltitudeOffsetInput = new QLineEdit;
    windAltitudeOffsetInput->setText("0v/t");

    windStylesLabel = new QLabel;
    windStylesLabel->setText("Style:");
    windStyles = new QComboBox;
    windStyles->addItem("Arrows");
    windStyles->addItem("Tufts");
    windStyles->addItem("Barbs");
    windStyles->addItem("Vectors");
    windStyles->addItem("T Vectors");
    windStyles->addItem("L Barbs");
    windStyles->addItem("Met Barbs");
    windStyles->addItem("SH Barbs");
    windStyles->addItem("LSH Barbs");

    windLegendLabel = new QLabel;
    windLegendLabel->setText("Show Legend:");
    windLegendSelect = new QCheckBox;

    windLayoutH1 = new QHBoxLayout;
    windLayoutH2 = new QHBoxLayout;
    windLayoutH3 = new QHBoxLayout;

    windLayoutV1 = new QVBoxLayout;
    windLayoutV2 = new QVBoxLayout;
    windLayoutV3 = new QVBoxLayout;
    windLayoutV4 = new QVBoxLayout;
    windLayoutVAll = new QVBoxLayout;

    windLayoutH1->addWidget(windSelectLabel);
    windLayoutH1->addWidget(windSelector);
    windLayoutH1->addStretch(0);

    windLayoutH2->addWidget(windUrlLabel);
    windLayoutH2->addWidget(windUrlInput);

    windLayoutV1->addWidget(windColorSelectText);
    windLayoutV1->addWidget(windColorSelect);
    windLayoutV1->addStretch(0);
    windLayoutV1->addWidget(windStylesLabel);
    windLayoutV1->addWidget(windStyles);
    windLayoutV1->addStretch(0);
    windLayoutV1->addWidget(windLegendLabel);
    windLayoutV1->addWidget(windLegendSelect);

    windLayoutV2->addWidget(windNumLabel);
    windLayoutV2->addWidget(windNumSlider);
    windLayoutV2->addSpacing(10);
    windLayoutV2->addStretch(0);
    windLayoutV2->addWidget(windWidthLabel);
    windLayoutV2->addWidget(windWidthSlider);
    windLayoutV2->addSpacing(10);
    windLayoutV2->addStretch(0);
    windLayoutV2->addWidget(windLengthLabel);
    windLayoutV2->addWidget(windLengthSlider);

    windLayoutV3->addWidget(UNameLabel);
    windLayoutV3->addWidget(UNameInput);
    windLayoutV3->addSpacing(10);
    windLayoutV3->addStretch(0);
    windLayoutV3->addWidget(VNameLabel);
    windLayoutV3->addWidget(VNameInput);
    windLayoutV3->addSpacing(10);
    windLayoutV3->addStretch(0);
    windLayoutV3->addWidget(WNameLabel);
    windLayoutV3->addWidget(WNameInput);

    windLayoutV4->addWidget(windTimeSlopLabel);
    windLayoutV4->addWidget(windTimeSlopInput);
    windLayoutV4->addSpacing(10);
    windLayoutV4->addStretch(0);
    windLayoutV4->addWidget(windTimeOffsetLabel);
    windLayoutV4->addWidget(windTimeOffsetInput);
    windLayoutV4->addSpacing(10);
    windLayoutV4->addStretch(0);
    windLayoutV4->addWidget(windAltitudeOffsetLabel);
    windLayoutV4->addWidget(windAltitudeOffsetInput);

    windLayoutH3->addLayout(windLayoutV1);
        line = new QFrame(windsConfig);
        line->setFrameShape(QFrame::VLine);
        line->setFrameShadow(QFrame::Raised);
        line->setLineWidth(5);
        windLayoutH3->addWidget(line);
    windLayoutH3->addLayout(windLayoutV2);
    windLayoutH3->addLayout(windLayoutV3);
    windLayoutH3->addLayout(windLayoutV4);
    windLayoutH3->addStretch(0);

    windLayoutVAll->addLayout(windLayoutH1);
    windLayoutVAll->addLayout(windLayoutH2);
    windLayoutVAll->addLayout(windLayoutH3);


    windsConfig->setLayout(windLayoutVAll);
}



void MainWindow::makeGridConfiguration()
{
    gridDataLayers = new QDialog(this);
    gridDataLayers->setWindowTitle("Grid Data Layers Configuration");

    gridLayerLabel = new QLabel;
    gridLayerLabel->setText("Grid Data Layer:");
    gridLayerSelector = new QComboBox;
    for(int i=0; i<16; i++)
    {   gridLayerSelector->addItem(QString::number(i+1));   }

    gridLabel = new QLabel;
    gridLabel->setText("Grid:");
    gridSelector = new QComboBox;
    gridSelector->addItem("KFTG DBZ");
    gridSelector->addItem("=NEXRAD=");
    gridSelector->addItem("=NEXRAD=");
    gridSelector->addItem("=NEXRAD=");
    gridSelector->addItem("=NEXRAD=");
    gridSelector->addItem("MANY MORE");

    gridUrlLabel = new QLabel;
    gridUrlLabel->setText("Url:");
    gridUrlInput = new QLineEdit;

    gridTopBotLabel = new QLabel;
    gridTopBotLabel->setText("Layers on:");
    gridTopBotSelector = new QComboBox;
    gridTopBotSelector->addItem("Top");
    gridTopBotSelector->addItem("Bottom");

    gridLegendLabel = new QLabel;
    gridLegendLabel->setText("Show Legend:");
    gridLegendBox = new QCheckBox;


    gridMinValueLabel = new QLabel;
    gridMinValueLabel->setText("Colors Min Value:");
    gridMinValueInput = new QLineEdit;
    gridMinValueInput->setText("-30");

    gridMaxValueLabel = new QLabel;
    gridMaxValueLabel->setText("Colors Max Value:");
    gridMaxValueInput = new QLineEdit;
    gridMaxValueInput->setText("100");

    gridDeltaLabel = new QLabel;
    gridDeltaLabel->setText("Delta:");
    gridDeltaInput = new QLineEdit;

    gridTimeSlopLabel = new QLabel;
    gridTimeSlopLabel->setText("Time Slop (min):");
    gridTimeSlopInput = new QLineEdit;
    gridTimeSlopInput->setText("100");

    gridTimeOffsetLabel = new QLabel;
    gridTimeOffsetLabel->setText("Time Offset (min);");
    gridTimeOffsetInput = new QLineEdit;
    gridTimeOffsetInput->setText("0");

    gridAltOffsetLabel = new QLabel;
    gridAltOffsetLabel->setText("Altitude Offset:");
    gridAltOffsetInput = new QLineEdit;
    gridAltOffsetInput->setText("0");

    gridColorMap = new QPushButton;
    gridColorMap->setText("Color Map");

    gridAutoUpdateLabel = new QLabel;
    gridAutoUpdateLabel->setText("Auto Update:");
    gridAutoUpdateBox = new QCheckBox;

    gridRequestCompositeLabel = new QLabel;
    gridRequestCompositeLabel->setText("Request Composite:");
    gridRequestCompositeBox = new QCheckBox;

    gridAutoscaleLabel = new QLabel;
    gridAutoscaleLabel->setText("Autoscale:");
    gridAutoscaleBox = new QCheckBox;

    gridColorScale = new QLabel;
    gridColorScale->setText("Colorscale: dbz.colors (editable?)");

    gridUnits = new QLabel;
    gridUnits->setText("Units: dBz (editable?)");

    gridRenderAsLabel = new QLabel;
    gridRenderAsLabel->setText("Render as:");
    gridRenderAsSelector = new QComboBox;
    gridRenderAsSelector->addItem("Polygons");
    gridRenderAsSelector->addItem("Contours");
    gridRenderAsSelector->addItem("Auto Contours");
    gridRenderAsSelector->addItem("Line Contours");



    gridWindowLayoutH1 = new QHBoxLayout;
    gridWindowLayoutH2 = new QHBoxLayout;
    gridWindowLayoutH3 = new QHBoxLayout;
    gridWindowLayoutH4 = new QHBoxLayout;
    gridWindowLayoutH5 = new QHBoxLayout;
    gridWindowLayoutH6 = new QHBoxLayout;
    gridWindowLayoutH7 = new QHBoxLayout;
    gridWindowLayoutH8 = new QHBoxLayout;
    gridWindowLayoutH9 = new QHBoxLayout;
    gridWindowLayoutH10 = new QHBoxLayout;
    gridWindowLayoutH11 = new QHBoxLayout;
    gridWindowLayoutH12 = new QHBoxLayout;
    gridWindowLayoutH13 = new QHBoxLayout;
    gridWindowLayoutH14 = new QHBoxLayout;
    gridWindowLayoutH15 = new QHBoxLayout;
    gridWindowLayoutH16 = new QHBoxLayout;
    gridWindowLayoutV1 = new QVBoxLayout;
    gridWindowLayoutV2 = new QVBoxLayout;
    gridWindowLayoutVAll = new QVBoxLayout;


    gridWindowLayoutH1->addWidget(gridLayerLabel);
    gridWindowLayoutH1->addWidget(gridLayerSelector);
    gridWindowLayoutH1->addWidget(gridLabel);
    gridWindowLayoutH1->addWidget(gridSelector);
    gridWindowLayoutH1->addStretch(0);

    gridWindowLayoutH2->addWidget(gridUrlLabel);
    gridWindowLayoutH2->addWidget(gridUrlInput);

    gridWindowLayoutH3->addWidget(gridUnits);
    gridWindowLayoutH3->addWidget(gridColorMap);
    gridWindowLayoutH3->addWidget(gridColorScale);


    gridWindowLayoutH4->addWidget(gridLegendLabel);
    gridWindowLayoutH4->addWidget(gridLegendBox);
    gridWindowLayoutH4->addStretch(0);
    gridWindowLayoutH5->addWidget(gridAutoUpdateLabel);
    gridWindowLayoutH5->addWidget(gridAutoUpdateBox);
    gridWindowLayoutH5->addStretch(0);
    gridWindowLayoutH6->addWidget(gridRequestCompositeLabel);
    gridWindowLayoutH6->addWidget(gridRequestCompositeBox);
    gridWindowLayoutH6->addStretch(0);
    gridWindowLayoutH7->addWidget(gridAutoscaleLabel);
    gridWindowLayoutH7->addWidget(gridAutoscaleBox);
    gridWindowLayoutH7->addStretch(0);
    gridWindowLayoutH8->addWidget(gridTopBotLabel);
    gridWindowLayoutH8->addWidget(gridTopBotSelector);
    gridWindowLayoutH8->addStretch(0);
    gridWindowLayoutH9->addWidget(gridRenderAsLabel);
    gridWindowLayoutH9->addWidget(gridRenderAsSelector);
    gridWindowLayoutH9->addStretch(0);
    gridWindowLayoutV1->addLayout(gridWindowLayoutH4);
    gridWindowLayoutV1->addLayout(gridWindowLayoutH5);
    gridWindowLayoutV1->addLayout(gridWindowLayoutH6);
    gridWindowLayoutV1->addLayout(gridWindowLayoutH7);
    gridWindowLayoutV1->addLayout(gridWindowLayoutH8);
    gridWindowLayoutV1->addLayout(gridWindowLayoutH9);

    gridWindowLayoutH10->addWidget(gridMinValueLabel);
    gridWindowLayoutH10->addStretch(0);
    gridWindowLayoutH10->addWidget(gridMinValueInput);
    gridWindowLayoutH11->addWidget(gridMaxValueLabel);
    gridWindowLayoutH11->addStretch(0);
    gridWindowLayoutH11->addWidget(gridMaxValueInput);
    gridWindowLayoutH12->addWidget(gridDeltaLabel);
    gridWindowLayoutH12->addStretch(0);
    gridWindowLayoutH12->addWidget(gridDeltaInput);
    gridWindowLayoutH13->addWidget(gridTimeSlopLabel);
    gridWindowLayoutH13->addStretch(0);
    gridWindowLayoutH13->addWidget(gridTimeSlopInput);
    gridWindowLayoutH14->addWidget(gridTimeOffsetLabel);
    gridWindowLayoutH14->addStretch(0);
    gridWindowLayoutH14->addWidget(gridTimeOffsetInput);
    gridWindowLayoutH15->addWidget(gridAltOffsetLabel);
    gridWindowLayoutH15->addStretch(0);
    gridWindowLayoutH15->addWidget(gridAltOffsetInput);
    gridWindowLayoutV2->addLayout(gridWindowLayoutH10);
    gridWindowLayoutV2->addLayout(gridWindowLayoutH11);
    gridWindowLayoutV2->addLayout(gridWindowLayoutH12);
    gridWindowLayoutV2->addLayout(gridWindowLayoutH13);
    gridWindowLayoutV2->addLayout(gridWindowLayoutH14);
    gridWindowLayoutV2->addLayout(gridWindowLayoutH15);

    gridWindowLayoutH16->addLayout(gridWindowLayoutV1);
    gridWindowLayoutH16->addLayout(gridWindowLayoutV2);

    gridWindowLayoutVAll->addLayout(gridWindowLayoutH1);
    gridWindowLayoutVAll->addLayout(gridWindowLayoutH2);
    gridWindowLayoutVAll->addLayout(gridWindowLayoutH3);
    gridWindowLayoutVAll->addLayout(gridWindowLayoutH16);

    gridDataLayers->setLayout(gridWindowLayoutVAll);
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
        movieDock->hide();
    }
    else {
        movieDock->show();
    }
}

void MainWindow::on_actionStatus_Window_triggered()
{
    QPoint p = MainWindow::pos();
    QSize  s = MainWindow::frameSize();
    Stat->move(p.x()+s.width(), p.y()+75);
    Stat->show();
    Stat->raise();
}

void MainWindow::on_actionVsection_triggered()
{
    QPoint p = MainWindow::pos();
    QSize  s = MainWindow::frameSize();
    Vsec->move(p.x()+s.width(), p.y());
    Vsec->show();
    Vsec->raise();
}

void MainWindow::on_actionZoom_Window_triggered()
{
    QPoint p = MainWindow::pos();
    QSize  s = MainWindow::frameSize();
    zoomOptions->move(p.x()+s.width(), p.y()+100);
    zoomOptions->show();
    zoomOptions->raise();
}


void MainWindow::on_actionValues_Cursor_toggled(bool arg1)
{
    if (arg1==0){
        valuesDisplay->hide();
    }
    else {
        QPoint p = MainWindow::pos();
        QSize  s = MainWindow::frameSize();
        valuesDisplay->move(p.x()+s.width(), p.y()+125);
        valuesDisplay->show();
        valuesDisplay->raise();
    }
}

void MainWindow::on_actionWind_Layer_triggered()
{
    QPoint p = MainWindow::pos();
    QSize  s = MainWindow::frameSize();
    windsConfig->move(p.x()+s.width(), p.y()+50);
    windsConfig->show();
    windsConfig->raise();
}

void MainWindow::on_actionData_Layers_triggered()
{
    QPoint p = MainWindow::pos();
    QSize  s = MainWindow::frameSize();
    gridDataLayers->move(p.x()+s.width(), p.y()+25);
    gridDataLayers->show();
    gridDataLayers->raise();
}



//cruddy zooms that need replacing
void MainWindow::on_actionZoomOut_triggered()
{

}

void MainWindow::on_actionZoomIn_triggered()
{

}


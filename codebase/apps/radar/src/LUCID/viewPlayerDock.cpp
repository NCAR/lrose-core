#include "viewPlayerDock.h"
#include "mainwindow.h"

//This file will control what the movie player dock widget looks like at the bottom of the main window
//It should work in tandem with contPlayerDock

viewPlayerDock::viewPlayerDock(QWidget *parent) : QDockWidget(parent)
{
    //class that creates the movie layer dock

    //set up the dockwidget
    QString movieTitle = "Movie Player";
    setWindowTitle(movieTitle);
    setAllowedAreas(Qt::DockWidgetAreas(Qt::TopDockWidgetArea |
                                                   Qt::BottomDockWidgetArea));
    setFeatures(QDockWidget::DockWidgetFeatures(QDockWidget::DockWidgetClosable |
                                                           QDockWidget::DockWidgetMovable |
                                                           QDockWidget::DockWidgetFloatable));

    //make widgets for movie player
    //TOP ROW------------------------------------------
    //frame label/indicator, indicator temprarily connected to slider posIndicator
    frameLabel = new QLabel;
    QString frameLabelText = "Frame";
    frameLabel->setText(frameLabelText);
    frameIndicator = new QLCDNumber;

    //player buttons, dummy at the moment
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

    //video tracking slider dummy at the moment, connected to frameIndicator and number of frames
    posIndicator = new QSlider(Qt::Horizontal);
    posIndicator->setMinimumWidth(400);
    posIndicator->setTickPosition(QSlider::TicksBelow);
    posIndicator->setTickInterval(1);

    //time and date display with dummy representative values
    //will represent selected time for playback loop
    frameTime = new QLabel;
    frameDate = new QLabel;

    //MIDDLE ROW--------------------------------------
    //time label/input
    timeLabel = new QLabel;
    timeInput = new QDateTimeEdit;
    QString timeLabelText = "Start Time & Date";
    timeLabel->setText(timeLabelText);

    //frame interval label/input
    frameIntervalLabel = new QLabel;
    frameIntervalInput = new QLineEdit;
    QString frameIntervalLabelText = "Frame Times(min)";
    frameIntervalLabel->setText(frameIntervalLabelText);
    frameIntervalInput->setMaximumWidth(100);

    //number of frames label/input
    numFramesLabel = new QLabel;
    numFramesInput = new QLineEdit;
    QString numFramesLabelText = "Number of Frames";
    numFramesLabel->setText(numFramesLabelText);
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

    sliderLabel = new QLabel;
    QString sliderImageText = R"(:/images/images/movieImage.png)";
    QImage sliderImage(sliderImageText);
    sliderLabel->setPixmap(QPixmap::fromImage(sliderImage));

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
    threeRows->addWidget(sliderLabel);

    //adding layout to the dock widget
    group->setLayout(threeRows);
    setWidget(group);
}


viewPlayerDock::~viewPlayerDock()
{
    //as of now, all pointers go into 'group', so that is all that needs to be deleted
    delete group;
}

















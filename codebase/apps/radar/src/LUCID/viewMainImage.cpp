#include "viewMainImage.h"

#include <iostream>

viewMainImage::viewMainImage(QString pic, QWidget *parent) : QWidget(parent)
{
    //------
    //this starts a timer that loops every second, it is currently being used for the clock on the Main Window
    //The timer can be used to for other things as well though.
    timer_1s = new QTimer(this);
    QObject::connect(timer_1s, SIGNAL(timeout()), this, SLOT(UpdateTime()));
    timer_1s->start(1000);
    //------

    //when viewMainImage is called, the main pic address is sent and used to create the main image, all just dummy stuff right now
    QImage image(pic);
    item = new QGraphicsPixmapItem(QPixmap::fromImage(image));
    scene = new QGraphicsScene;
    scene->addItem(item);
    view = new QGraphicsView(scene);
    view->setMouseTracking(true);

    //scale image dummy
    QString scalePic = R"(:/images/images/dBzScale.png)";
    scale = new QLabel;
    QPixmap *scaleImg = new QPixmap(scalePic);

    //will be used to show the UTC time
    //see viewMainImage::UpdateTime()
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

    setLayout(timeLayout);

    setMouseTracking(true);
}

//temporary funtion to be deleted for playing around with stuff
void viewMainImage::Tester()
{
    std::cout << "worked" << std::endl;
}

//function is called every second from the connection with timer_1s
//updates the currentTime label with current UTC time
void viewMainImage::UpdateTime()
{
    QDateTime local(QDateTime::currentDateTime());
    QDateTime UTC(local.toUTC());
    QString temp = QLocale("en_EN").toString(UTC, "dddd MMMM dd yyyy  hh:mm:ss (UTC)");
    currentTime->setText(temp);
}


viewMainImage::~viewMainImage()
{
    //as of now, all pointers go into timeLayout, so that is all that needs to be deleted
    delete timeLayout;
}




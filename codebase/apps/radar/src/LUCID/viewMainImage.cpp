#include "viewMainImage.h"
#include <iostream>

//this class should control how information is displayed in the main window of LUCID
//It should only control the central image, including the radar display, dBz scale, and time display.
//This should work in tandem with contMainImage, which controls what info is displayed.

//FOR NOW, this class is only displaying dummy images. This will need to be changed considerably
//in the way this class functions. In order to imitate the way hawkeye displays images, you will need to use
//QImage
//QPaint
//QBrushes
//QRect
//QTransform
//QPointF



viewMainImage::viewMainImage(QLabel *currentTime, QString pic, QWidget *parent) : QWidget(parent)
{

    mainImage = new QLabel;
    pixie.load(pic);
    mainImage->setPixmap(pixie);
    scrollArea = new QScrollArea;
    scrollArea->setWidget(mainImage);


    QString scalePic = R"(:/images/images/dBzScale.png)";
    scale = new QLabel;
    QPixmap *scaleImg = new QPixmap(scalePic);

    scale->setPixmap(*scaleImg);
    scale->setScaledContents(1);
    scale->setMinimumSize(35, 1);
    scale->setMaximumSize(36, 10000);

    mainLayout = new QHBoxLayout;
    mainLayout->addWidget(scrollArea);

    mainLayout->addWidget(scale);
    QWidget *w = new QWidget;
    w->setLayout(mainLayout);

    timeLayout = new QVBoxLayout;
    timeLayout->addWidget(currentTime);
    timeLayout->addWidget(w);

    setLayout(timeLayout);

    //mousetracking settings for value cursor, this enables the mouse tracking to work
    //for displaying coordinated next to the cursor for values. Right now these settings
    //feel clunky and will most likely be changed by the end.
    this->setMouseTracking(true);
    currentTime->setMouseTracking(0);
    w->setMouseTracking(true);
    scale->setMouseTracking(0);
    scrollArea->setMouseTracking(true);
    mainImage->setMouseTracking(true);
}


viewMainImage::~viewMainImage()
{
    //as of now, all pointers go into timeLayout, so that is all that needs to be deleted
    delete timeLayout;
}

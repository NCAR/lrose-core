#include "viewMainImage.h"

#include <iostream>

viewMainImage::viewMainImage(QLabel *currentTime, QString pic, QWidget *parent) : QWidget(parent)
{
    QImage recievedImage(pic);
    image = recievedImage;
    mainImage = new QLabel;
    pixie.load(pic);
    //mainImage->setPixmap(QPixmap::fromImage(image));
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
    //mousetracking settings for value cursor
    this->setMouseTracking(true);
    currentTime->setMouseTracking(0);
    w->setMouseTracking(true);
    scale->setMouseTracking(0);
    scrollArea->setMouseTracking(true);
    mainImage->setMouseTracking(true);
}


//temporary funtion to be deleted for playing around with stuff
void viewMainImage::Tester()
{
    std::cout << "worked" << std::endl;
}


viewMainImage::~viewMainImage()
{
    //as of now, all pointers go into timeLayout, so that is all that needs to be deleted
    //delete timeLayout;
}

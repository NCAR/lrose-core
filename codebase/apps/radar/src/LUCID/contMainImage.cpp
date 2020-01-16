#include "contMainImage.h"
#include "viewMainImage.h"

//this file should be the controller for what information is displayed in the
//main window of LUCID. It should only control the central image, including
//the radar display, dBz scale, and time display.
//This should work in tandem with viewMainImage, which controls how the images
//are displayed.

//FOR NOW, this class is only controlling the time displayed in the main window.

contMainImage::contMainImage(QString pic, QWidget *parent) : QWidget(parent)
{
    timer_1s = new QTimer();
    //the line below is why I had to make contMainImage a QWidget, the error is
    //"no matching member function for call to 'connect'" when this class is not a QWidget
    QObject::connect(timer_1s, SIGNAL(timeout()), this, SLOT(UpdateTime()));
    timer_1s->start(1000);
    currentTime = new QLabel;
    viewer = new  viewMainImage(currentTime, pic, this);
}

void contMainImage::UpdateTime()
{
    QDateTime local(QDateTime::currentDateTime());
    QDateTime UTC(local.toUTC());
    QString temp = QLocale("en_EN").toString(UTC, "dddd MMMM dd yyyy  hh:mm:ss (UTC)");
    currentTime->setText(temp);
}

contMainImage::~contMainImage()
{
    delete timer_1s;
    delete currentTime;
    delete viewer;
}




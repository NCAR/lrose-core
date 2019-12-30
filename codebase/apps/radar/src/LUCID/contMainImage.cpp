#include "contMainImage.h"
#include "viewMainImage.h"

contMainImage::contMainImage(QString pic, QWidget *parent) : QWidget(parent)
{
    timer_1s = new QTimer();
    //the line below is why I had to make contMainImage a QWidget, the error is
    //"no matching member function for call to 'connect'"
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



























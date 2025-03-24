#ifndef CONTMAINIMAGE_H
#define CONTMAINIMAGE_H
#include <QDateTime>
#include <QLabel>
#include <QLocale>
#include <QTimer>
#include <QWidget>
#include <QScrollArea>
#include <QHBoxLayout>

#include "viewMainImage.h"

class contMainImage : public QWidget
{
    Q_OBJECT
public slots:

private slots:
    void UpdateTime();

public:
    explicit contMainImage(QString pic, QWidget *parent = nullptr);
    ~contMainImage();
    QLabel *currentTime;
    viewMainImage *viewer;

private:
    //for reseting the clock in main window every second
    QTimer *timer_1s;
};

#endif // CONTMAINIMAGE_H



















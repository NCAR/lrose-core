#ifndef VIEWMAINIMAGE_H
#define VIEWMAINIMAGE_H



#include <QWidget>
#include <QLabel>
#include <QPixmap>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include <QTimer>
#include <QDateTime>
#include <QLocale>


class viewMainImage : public QWidget
{
    Q_OBJECT
public:
    explicit viewMainImage(QString pic, QWidget *parent = nullptr);
    ~viewMainImage();
    void Tester();



signals:

public slots:


private slots:
    void UpdateTime();

private:
    //for reseting the clock in main window every second
    QTimer *timer_1s;


    QLabel *mainImage, *scale, *currentTime, *currentDate;
    QPixmap *img, *scaleImg;
    QHBoxLayout *mainLayout, *dateTimeLayout;
    QVBoxLayout *timeLayout;
    QWidget *placeholderWidget;
    QGraphicsView *view;
    QGraphicsScene *scene;
    QGraphicsPixmapItem *item;
};

#endif // VIEWMAINIMAGE_H

#ifndef VIEWMAINIMAGE_H
#define VIEWMAINIMAGE_H
#include <QDateTime>
#include <QLabel>
#include <QLocale>
#include <QTimer>
#include <QWidget>
#include <QScrollArea>
#include <QHBoxLayout>

class viewMainImage : public QWidget
{
    Q_OBJECT
public:
    explicit viewMainImage(QLabel *currentTime, QString pic, QWidget *parent = nullptr);
    ~viewMainImage();
    QLabel *mainImage;
    QScrollArea *scrollArea;
    QVBoxLayout *timeLayout;

signals:


public slots:


private slots:


protected:


private:
    QImage image;
    QPixmap pixie;
    QLabel *scale;
    QHBoxLayout *mainLayout;

};

#endif // VIEWMAINIMAGE_H

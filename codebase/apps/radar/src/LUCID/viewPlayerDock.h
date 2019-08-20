#ifndef VIEWPLAYERDOCK_H
#define VIEWPLAYERDOCK_H

#include <QDialog>
#include <QDockWidget>
#include <QSlider>
#include <QLCDNumber>
#include <QLineEdit>
#include <QLabel>
#include <QToolButton>
#include <QDateTimeEdit>
#include <QComboBox>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QFrame>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>





class viewPlayerDock : public QDockWidget
{
    Q_OBJECT
public:
    explicit viewPlayerDock(QWidget *parent = nullptr);
    ~viewPlayerDock();

signals:

public slots:

private slots:
    void frameChanged();

private:




    QDockWidget *movieDock;
    QSlider *posIndicator;
    QLCDNumber *frameIndicator;
    QLineEdit *frameIntervalInput;
    QLineEdit *numFramesInput;
    QLabel *frameLabel, *frameTime, *frameDate, *timeLabel, *frameIntervalLabel, *numFramesLabel, *playbackLabel, *delayLabel;
    QToolButton *rwd, *play, *pause, *fwd;
    QDateTimeEdit *timeInput;
    QComboBox *playback, *delay, *realArchive, *loopSweep;
    QHBoxLayout *topRow, *midRow, *botRow;
    QVBoxLayout *threeRows;
    QGroupBox *group;
    QFrame *line;

    QGraphicsView *sliderView;
    QGraphicsScene *sliderScene;
    QGraphicsPixmapItem *sliderItem;

};

#endif // VIEWPLAYERDOCK_H

#ifndef VIEWPLAYERDOCK_H
#define VIEWPLAYERDOCK_H

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

class viewPlayerDock : public QDockWidget
{
    Q_OBJECT
public:
    explicit viewPlayerDock(QWidget *parent = nullptr);
    ~viewPlayerDock();

    QLabel *frameLabel, *frameTime, *frameDate, *timeLabel, *frameIntervalLabel, *numFramesLabel, *playbackLabel, *delayLabel, *sliderLabel;
    QToolButton *rwd, *play, *pause, *fwd;
    QSlider *posIndicator;
    QLineEdit *frameIntervalInput, *numFramesInput;
    QDateTimeEdit *timeInput;
    QComboBox *playback, *delay, *realArchive, *loopSweep;
    QLCDNumber *frameIndicator;

signals:

public slots:

private slots:

private:
    QDockWidget *movieDock;
    QHBoxLayout *topRow, *midRow, *botRow;
    QVBoxLayout *threeRows;
    QGroupBox *group;
    QFrame *line;
};

#endif // VIEWPLAYERDOCK_H

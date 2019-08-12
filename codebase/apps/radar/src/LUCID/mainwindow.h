#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "v_section.h"
#include "statusdialog.h"

#include <cmath>

#include <QMainWindow>
//#include <QToolTip>
#include <QLabel>
#include <QDockWidget>
#include <QPushButton>
#include <QGroupBox>
#include <QLCDNumber>
#include <QSlider>
#include <QLineEdit>
#include <QDateTimeEdit>
#include <QToolButton>
#include <QComboBox>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QSpinBox>
#include <QString>
#include <QToolBar>
#include <QTimer>
#include <QTime>

#include <QGraphicsScene>
#include <QGraphicsView>
#include <QMouseEvent>



namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:

    void frameChanged();

    void on_actionFields_toggled(bool arg1);
    void on_actionProducts_toggled(bool arg1);
    void on_actionMaps_toggled(bool arg1);
    void on_actionMovie_Player_toggled(bool arg1);
    void on_actionZoomOut_triggered();
    void on_actionZoomIn_triggered();
    void on_actionStatus_Window_triggered();
    void on_actionVsection_triggered();

    void UpdateTime();




private:
    Ui::MainWindow *ui;

    //for showing coordinated of cursor in main window
    void mouseMoveEvent(QMouseEvent* event);

    //for reseting the clock in main window every second
    QTimer *timer_1s;

    //to get the other windows going in MainWindow()
    V_section *Vsec;
    StatusDialog *Stat;

    //main image starter
    void startImage();
    QLabel *mainImage, *scale, *currentTime, *currentDate;
    QPixmap *img, *scaleImg;
    QHBoxLayout *mainLayout, *dateTimeLayout;
    QVBoxLayout *timeLayout;
    QWidget *placeholderWidget;

    QGraphicsView *view;
    QGraphicsScene *scene;
    QGraphicsPixmapItem *item;



    //toolbar maker
    void startToolBars();
    QToolBar *fields, *products, *maps;

    //spacermaker
    void toolBarSpacers();
    QWidget *emptySpacer;

    //field dock maker
    void fieldDockMaker();
    QDockWidget *fieldDock;
    QListWidget *fieldList;
    QVBoxLayout *fieldLayout;
    QPushButton *field[3][10];
    QGridLayout *grid;
    QGroupBox *fieldGroup;

    //moviedock widget
    void movieLooperDock();
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
};

#endif // MAINWINDOW_H

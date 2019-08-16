#ifndef MAINWINDOW_H
#define MAINWINDOW_H


#include "viewWindDialog.h"
#include "viewGridConfigDialog.h"
#include "viewVsection.h"
#include "viewStatusDialog.h"

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


    void on_actionZoom_Window_triggered();

    void on_actionValues_Cursor_toggled(bool arg1);

    void on_actionWind_Layer_triggered();

    void on_actionData_Layers_triggered();

private:
    Ui::MainWindow *ui;

    //for showing coordinated of cursor in main window
    void mouseMoveEvent(QMouseEvent* event);

    //for reseting the clock in main window every second
    QTimer *timer_1s;

    //to get the other windows going in MainWindow()
    viewVsection *Vsec;
    viewStatusDialog *Stat;
    viewWindDialog *windDialog;
    viewGridConfigDialog *gridConfigDialog;


   // windDialogView *windDialog;

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
    QToolBar *products, *maps;

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
    QFrame *line;

    QGraphicsView *sliderView;
    QGraphicsScene *sliderScene;
    QGraphicsPixmapItem *sliderItem;


    void makeZoomOptions();
    QDialog *zoomOptions;
    QPushButton *zoom10, *zoom100, *zoom1000, *zoomSaved, *zoomSaved2, *zoomReset;
    QVBoxLayout *zoomLayout;

    void makeValuesDisplay();
    QDialog *valuesDisplay;
    QLabel *valueLabel1, *valueLabel2, *valueLabel3, *valueLabel4;
    QTextBrowser *valueOf1, *valueOf2, *valueOf3, *valueOf4;
    QHBoxLayout *valueCombo1, *valueCombo2, *valueCombo3, *valueCombo4;
    QVBoxLayout *valuesLayout;





};

#endif // MAINWINDOW_H

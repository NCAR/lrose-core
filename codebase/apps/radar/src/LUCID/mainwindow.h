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


    void makeWindsConfiguration();
    QDialog *windsConfig;
    QLabel *windSelectLabel, *windUrlLabel, *windColorSelectText, *windNumLabel, *windWidthLabel, *windLengthLabel,
            *UNameLabel, *VNameLabel, *WNameLabel,*windTimeSlopLabel, *windTimeOffsetLabel, *windAltitudeOffsetLabel,
            *windStylesLabel, *windLegendLabel;
    QComboBox *windSelector, *windColorSelect, *windStyles;
    QLineEdit *windUrlInput, *UNameInput, *VNameInput, *WNameInput, *windTimeSlopInput, *windTimeOffsetInput,
            *windAltitudeOffsetInput;
    QSlider *windNumSlider, *windWidthSlider, *windLengthSlider;
    QCheckBox *windLegendSelect;
    QHBoxLayout *windLayoutH1, *windLayoutH2, *windLayoutH3;
    QVBoxLayout *windLayoutV1, *windLayoutV2, *windLayoutV3, *windLayoutV4, *windLayoutVAll;

    void makeGridConfiguration();
    QDialog *gridDataLayers;
    QLabel *gridLayerLabel, *gridLabel, *gridUrlLabel, *gridTopBotLabel, *gridLegendLabel, *gridMinValueLabel,
            *gridMaxValueLabel, *gridDeltaLabel, *gridTimeSlopLabel, *gridTimeOffsetLabel, *gridAltOffsetLabel,
            *gridAutoUpdateLabel, *gridRequestCompositeLabel, *gridAutoscaleLabel, *gridColorScale, *gridUnits,
            *gridRenderAsLabel;
    QComboBox *gridLayerSelector, *gridSelector, *gridTopBotSelector, *gridRenderAsSelector;
    QLineEdit *gridUrlInput, *gridMinValueInput, *gridMaxValueInput, *gridDeltaInput, *gridTimeSlopInput,
            *gridTimeOffsetInput, *gridAltOffsetInput, *gridColorMapInput;
    QCheckBox *gridLegendBox, *gridAutoUpdateBox, *gridRequestCompositeBox, *gridAutoscaleBox;
    QPushButton *gridColorMap;
    QHBoxLayout *gridWindowLayoutH1, *gridWindowLayoutH2, *gridWindowLayoutH3, *gridWindowLayoutH4,
            *gridWindowLayoutH5, *gridWindowLayoutH6, *gridWindowLayoutH7, *gridWindowLayoutH8,
            *gridWindowLayoutH9, *gridWindowLayoutH10, *gridWindowLayoutH11, *gridWindowLayoutH12,
            *gridWindowLayoutH13, *gridWindowLayoutH14, *gridWindowLayoutH15, *gridWindowLayoutH16;
    QVBoxLayout *gridWindowLayoutV1, *gridWindowLayoutV2, *gridWindowLayoutV3, *gridWindowLayoutV4, *gridWindowLayoutVAll;



};

#endif // MAINWINDOW_H

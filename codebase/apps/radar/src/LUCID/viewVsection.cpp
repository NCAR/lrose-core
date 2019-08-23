#include "viewVsection.h"

viewVsection::viewVsection(QWidget *parent) : QDialog(parent)
{
    //eventually this widget will handle Vsection. It should get changed A LOT
    //right now it is just a dummy representation with a bad graph

    //initialize layouts
    group = new QGroupBox;
    buttonLayout = new QHBoxLayout();
    mainLayout = new QVBoxLayout();

    //initialize buttons
    clear = new QPushButton(tr("Clear"));
    close = new QPushButton(tr("Close"));
    //initialize top label and edit
    labelTop = new QLabel(tr("Scale Top"));
    editTop = new QLineEdit;
    //initialize base label and edit
    labelBase = new QLabel(tr("Scale Base"));
    editBase = new QLineEdit;
    //make connections for the 2 buttons
    connect(close, SIGNAL(clicked()), this, SLOT(close()));
    connect(clear, SIGNAL(clicked()), editTop, SLOT(clear()));
    connect(clear, SIGNAL(clicked()), editBase, SLOT(clear()));

    //add everything that goes in top row to the Hlayout
    buttonLayout->addWidget(labelTop);
    buttonLayout->addWidget(editTop);
    buttonLayout->addWidget(labelBase);
    buttonLayout->addWidget(editBase);
    buttonLayout->insertStretch(5,3000);
    buttonLayout->addWidget(clear);
    buttonLayout->addWidget(close);
    group->setLayout(buttonLayout);


    //DUMMY GRAPH-----------------------------------------------------
    //make dummy numbers for the dummy graph
    series = new QLineSeries();
    series1 = new QLineSeries();
    series2 = new QLineSeries();
    series3 = new QLineSeries();
    for(int i=0; i<40; i++)
    {
        *series << QPointF(i,(i/15)+i);
        *series1 << QPointF(i,(i/3)+i);
        *series2 << QPointF(i,(i/30)+i);
        *series3 << QPointF(i,(i/9)+i);
    }
    //make dummy chart
    chart = new QChart();
    //add dummy numbers to dummy chart
    chart->addSeries(series);
    chart->addSeries(series1);
    chart->addSeries(series2);
    chart->addSeries(series3);
    QStringList categories;
    categories << "2013" << "2014" << "2015" << "2016" << "2017" << "2018";
    //mod looks of dummy chart
    chart->setTitle("Vsection");
    chart->setAnimationOptions(QChart::AllAnimations);
    chart->createDefaultAxes();
    chart->legend()->setVisible(true);
    chart->legend()->setAlignment(Qt::AlignBottom);
    chart->setBackgroundVisible(1);
    chart->setTheme(QChart::ChartThemeDark);
    //END DUMMY GRAPH--------------------------------------------------


    //initialize chartview to show dummy chart
    chartView = new QChartView(chart);
    //add top row group, and chartview to the main Vlayout
    mainLayout->addWidget(group);
    mainLayout->addWidget(chartView);
    setLayout(mainLayout);
    resize(825, 800);
}

viewVsection::~viewVsection()
{
    //as of now, all pointers go into 'mainLayout', so that is all that needs to be deleted
    delete mainLayout;
}

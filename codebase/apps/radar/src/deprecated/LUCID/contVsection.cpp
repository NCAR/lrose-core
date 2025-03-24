#include "contVsection.h"

//This file controls what the vsection window will display
//it should work in tandem with viewVsection
//for now, it is only displaying a dummy graph,
//however the button connections should still be valid

contVsection::contVsection()
{
    VsectionViewer = new viewVsection;

    //make connections for the 2 buttons
    connect(VsectionViewer->close, SIGNAL(clicked()), VsectionViewer, SLOT(close()));
    connect(VsectionViewer->clear, SIGNAL(clicked()), VsectionViewer->editTop, SLOT(clear()));
    connect(VsectionViewer->clear, SIGNAL(clicked()), VsectionViewer->editBase, SLOT(clear()));

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
    //add dummy numbers to dummy chart
    VsectionViewer->chart->addSeries(series);
    VsectionViewer->chart->addSeries(series1);
    VsectionViewer->chart->addSeries(series2);
    VsectionViewer->chart->addSeries(series3);
    //mod looks of dummy chart
    VsectionViewer->chart->setTitle("Vsection");
    VsectionViewer->chart->setAnimationOptions(QChart::AllAnimations);
    VsectionViewer->chart->createDefaultAxes();
    VsectionViewer->chart->legend()->setVisible(true);
    VsectionViewer->chart->legend()->setAlignment(Qt::AlignBottom);
    VsectionViewer->chart->setBackgroundVisible(1);
    VsectionViewer->chart->setTheme(QChart::ChartThemeDark);
    //END DUMMY GRAPH--------------------------------------------------
}

contVsection::~contVsection()
{
    delete series;
    delete series1;
    delete series2;
    delete series3;
    delete VsectionViewer;
}














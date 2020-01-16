#include "viewVsection.h"

//This file controls what the vsection window will look like.
//it should work in tandem with contVsection

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

    //add everything that goes in top row to the Hlayout
    buttonLayout->addWidget(labelTop);
    buttonLayout->addWidget(editTop);
    buttonLayout->addWidget(labelBase);
    buttonLayout->addWidget(editBase);
    buttonLayout->insertStretch(5,3000);
    buttonLayout->addWidget(clear);
    buttonLayout->addWidget(close);
    group->setLayout(buttonLayout);

    chart = new QChart();
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

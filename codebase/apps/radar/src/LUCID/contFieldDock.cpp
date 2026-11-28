#include "contFieldDock.h"

//this file should be the controller for the field dock.
//it will have to decide how many, and what fields to display for the user,


//for now, it is just displaying dummy information
contFieldDock::contFieldDock()
{
    //initialize field dock and specify properties
    fieldDock = new QDockWidget;
    QString fieldTitle = "Field Options";
    fieldDock->setWindowTitle(fieldTitle);
    fieldDock->setAllowedAreas(Qt::DockWidgetAreas(Qt::RightDockWidgetArea |
                                                   Qt::LeftDockWidgetArea));
    fieldDock->setFeatures(QDockWidget::DockWidgetFeatures(QDockWidget::DockWidgetClosable |
                                                           QDockWidget::DockWidgetMovable |
                                                           QDockWidget::DockWidgetFloatable));

    //create list widget as a dummy for that list thing in the fields window
    fieldList = new QListWidget;
    for(int i=0; i<10; i++)
    {
        fieldList->addItems(QStringList() << "Field Option");
    }

    //make grid of pushbuttons.
    //should be remade with a vector or something better when this becomes real.
    int count = 0;
    for(int i=0; i<3; i++)
    {
        for(int j=0; j<10; j++)
        {
            count++;
            //field[i][j] = new QPushButton(tr("field ")+QString::number(count));
            field[i][j] = new QPushButton(("FEEld ")+QString::number(count));
        }
    }
    grid = new QGridLayout;
    for(int i=0; i<2; i++)
    {
        for(int j=0; j<10; j++)
        {
            grid->addWidget(field[i][j],j,i);
        }
    }

    //add list, and buttons to layout
    fieldLayout = new QVBoxLayout();
    fieldLayout->addWidget(fieldList);
    fieldLayout->addLayout(grid);

    //make grid and add layout to it to add to the widget
    fieldGroup = new QGroupBox;
    fieldGroup->setLayout(fieldLayout);
    fieldDock->setWidget(fieldGroup);
}


contFieldDock::~contFieldDock()
{
    delete fieldList;
    for (int i=0; i<10; i++)
    {
        delete field[1][i];
        delete field[2][i];
        delete field[3][i];
    }
    delete grid;
    delete fieldLayout;
    delete fieldGroup;
    delete fieldDock;
}

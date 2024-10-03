#include "viewStatusDialog.h"


//This file will control what the status window looks like.
//It should work in tandem with contStatusDialog

viewStatusDialog::viewStatusDialog(QWidget *parent) : QDialog(parent)
{
    setWindowTitle(tr("Status Window"));

    //initialize layouts
    group = new QGroupBox;
    buttonLayout = new QHBoxLayout;
    mainLayout = new QVBoxLayout;

    //make buttons
    clear = new QPushButton(tr("Clear"));
    close = new QPushButton(tr("Close"));

    //and add them to the Hlayout
    buttonLayout->insertStretch(0,0);
    buttonLayout->addWidget(clear);
    buttonLayout->addWidget(close);
    group->setLayout(buttonLayout);

    //initialize and make status window list widget
    statuses = new QListWidget(this);

    //add buttons, and list widget to Vlayout and set to the window layout
    mainLayout->addWidget(group);
    mainLayout->addWidget(statuses);
    setLayout(mainLayout);
}

viewStatusDialog::~viewStatusDialog()
{
    //as of now, all pointers go into 'mainLayout', so that is all that needs to be deleted
    delete mainLayout;
}

#include "viewStatusDialog.h"

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
    for(int i=0; i<40; i++)
    {
        statuses->addItems(QStringList() << "Something is happening with the weather");
        statuses->addItems(QStringList() << "Or maybe something is happening with CIDD");
        statuses->addItems(QStringList() << "Hopefully this becomes something");
        statuses->addItems(QStringList() << "Will the status of LUCID's progress be included");
        statuses->addItems(QStringList() << "   in this status window? Doubt");
    }

    //connect the 2 buttons
    connect(clear, SIGNAL(clicked()), statuses, SLOT(clear()));
    connect(close, SIGNAL(clicked()), this, SLOT(reject()));

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

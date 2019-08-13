#include "statusdialog.h"
#include "ui_statusdialog.h"


StatusDialog::StatusDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::StatusDialog)
{
    ui->setupUi(this);
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
    }

    //connect the 2 buttons
    connect(clear, SIGNAL(clicked()), statuses, SLOT(clear()));
    connect(close, SIGNAL(clicked()), this, SLOT(reject()));

    //add buttons, and list widget to Vlayout and set to the window layout
    mainLayout->addWidget(group);
    mainLayout->addWidget(statuses);
    setLayout(mainLayout);
}

StatusDialog::~StatusDialog()
{
    delete ui;
}

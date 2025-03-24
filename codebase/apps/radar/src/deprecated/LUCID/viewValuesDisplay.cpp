#include "viewValuesDisplay.h"

//This file controlls how the values dialog window looks for the user
//It should work in tandem with contValuesDisplay

viewValuesDisplay::viewValuesDisplay(QWidget *parent) : QDialog(parent)
{
    //Eventually this widget will display the various data values under the cursor as it is moved over the display
    setWindowTitle("Values");

    //labels
    valueLabel1 = new QLabel;
    valueLabel1->setText("Value of 1: ");
    valueLabel2 = new QLabel;
    valueLabel2->setText("Value of 2: ");
    valueLabel3 = new QLabel;
    valueLabel3->setText("Value of 3: ");
    valueLabel4 = new QLabel;
    valueLabel4->setText("Value of 4: ");

    //text browsers with dummy numbers
    valueOf1 = new QTextBrowser;
    valueOf1->setText("0.01");
    valueOf1->setMaximumHeight(26);
    valueOf2 = new QTextBrowser;
    valueOf2->setText("0.02");
    valueOf2->setMaximumHeight(26);
    valueOf3 = new QTextBrowser;
    valueOf3->setText("0.03");
    valueOf3->setMaximumHeight(26);
    valueOf4 = new QTextBrowser;
    valueOf4->setText("0.04");
    valueOf4->setMaximumHeight(26);

    //layouts
    valueCombo1 = new QHBoxLayout;
    valueCombo1->addWidget(valueLabel1);
    valueCombo1->addWidget(valueOf1);
    valueCombo2 = new QHBoxLayout;
    valueCombo2->addWidget(valueLabel2);
    valueCombo2->addWidget(valueOf2);
    valueCombo3 = new QHBoxLayout;
    valueCombo3->addWidget(valueLabel3);
    valueCombo3->addWidget(valueOf3);
    valueCombo4 = new QHBoxLayout;
    valueCombo4->addWidget(valueLabel4);
    valueCombo4->addWidget(valueOf4);

    valuesLayout = new QVBoxLayout;
    valuesLayout->addLayout(valueCombo1);
    valuesLayout->addLayout(valueCombo2);
    valuesLayout->addLayout(valueCombo3);
    valuesLayout->addLayout(valueCombo4);

    setLayout(valuesLayout);
}


viewValuesDisplay::~viewValuesDisplay()
{
    //as of now, all pointers go into 'valuesLayout', so that is all that needs to be deleted
    delete valuesLayout;
}

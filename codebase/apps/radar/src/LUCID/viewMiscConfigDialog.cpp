#include "viewMiscConfigDialog.h"


//This file will control how info is diplayed in the miscellaneous
//configurations dialog window. It should work in tandem with contMiscConfigDialog

viewMiscConfigDialog::viewMiscConfigDialog(QWidget *parent) : QDialog(parent)
{
    setWindowTitle("Miscellaneous Configurations");

    symprodLabel = new QLabel;
    symprodLabel->setText("Symprod:");
    symprodSelector = new QComboBox;
    symprodSelector->addItem("RHI-ticks");
    symprodSelector->addItem("RHI-label");

    constraintsLabel = new QLabel;
    constraintsLabel->setText("Constraints:");
    constraintsSelector = new QComboBox;
    constraintsSelector->addItem("Render All");
    constraintsSelector->addItem("Render All Valid");
    constraintsSelector->addItem("Render Valid in Last Frame");
    constraintsSelector->addItem("Render Latest in Frame");
    constraintsSelector->addItem("Render Latest in Loop");
    constraintsSelector->addItem("Render First After Data Time");
    constraintsSelector->addItem("Render First Before Data Time");
    constraintsSelector->addItem("Render All Before Data Time");
    constraintsSelector->addItem("Render All After Data Time");

    urlLabel = new QLabel;
    urlLabel->setText("URL:");
    urlInput = new QLineEdit;

    dataTypeLabel = new QLabel;
    dataTypeLabel->setText("Data Type:");
    dataTypeBox = new QCheckBox;

    allowBeforeLabel = new QLabel;
    allowBeforeLabel->setText("Allow Before:");
    allowBeforeInput = new QLineEdit;

    allowAfterLabel = new QLabel;
    allowAfterLabel->setText("After:");
    allowAfterInput = new QLineEdit;

    textThresholdLabel = new QLabel;
    textThresholdLabel->setText("Text Threshold:");
    textThresholdInput = new QLineEdit;

    topoUrlLabel = new QLabel;
    topoUrlLabel->setText("Topo URL:");
    topoUrlInput = new QLineEdit;

    landUseLabel = new QLabel;
    landUseLabel->setText("Land Use URL:");
    landUseInput = new QLineEdit;


    miscLayoutH1 = new QHBoxLayout;
    miscLayoutH2 = new QHBoxLayout;
    miscLayoutH3 = new QHBoxLayout;
    miscLayoutH4 = new QHBoxLayout;
    miscLayoutH5 = new QHBoxLayout;
    miscLayoutH6 = new QHBoxLayout;
    miscLayoutH7 = new QHBoxLayout;

    miscLayoutV1 = new QVBoxLayout;

    miscLayoutH1->addWidget(symprodLabel);
    miscLayoutH1->addWidget(symprodSelector);
    miscLayoutH1->addStretch(0);
    miscLayoutH1->addWidget(constraintsLabel);
    miscLayoutH1->addWidget(constraintsSelector);

    miscLayoutH2->addWidget(urlLabel);
    miscLayoutH2->addWidget(urlInput);

    miscLayoutH3->addWidget(dataTypeLabel);
    miscLayoutH3->addWidget(dataTypeBox);
    miscLayoutH3->addStretch(0);

    miscLayoutH4->addWidget(textThresholdLabel);
    miscLayoutH4->addWidget(textThresholdInput);
    miscLayoutH4->addStretch(0);

    miscLayoutH5->addWidget(allowBeforeLabel);
    miscLayoutH5->addWidget(allowBeforeInput);
    miscLayoutH5->addWidget(allowAfterLabel);
    miscLayoutH5->addWidget(allowAfterInput);
    miscLayoutH5->addStretch(0);

    line = new QFrame();
    line->setFrameShape(QFrame::HLine);
    line->setLineWidth(3);

    miscLayoutH6->addWidget(topoUrlLabel);
    miscLayoutH6->addWidget(topoUrlInput);

    miscLayoutH7->addWidget(landUseLabel);
    miscLayoutH7->addWidget(landUseInput);


    miscLayoutV1->addLayout(miscLayoutH1);
    miscLayoutV1->addLayout(miscLayoutH2);
    miscLayoutV1->addLayout(miscLayoutH3);
    miscLayoutV1->addLayout(miscLayoutH4);
    miscLayoutV1->addLayout(miscLayoutH5);
    miscLayoutV1->addWidget(line);
    miscLayoutV1->addLayout(miscLayoutH6);
    miscLayoutV1->addLayout(miscLayoutH7);
    setLayout(miscLayoutV1);
}

viewMiscConfigDialog::~viewMiscConfigDialog()
{
    delete miscLayoutH1;
}

#include "viewMiscConfigDialog.h"

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
    miscLayoutH3->addSpacing(10);
    miscLayoutH3->addWidget(textThresholdLabel);
    miscLayoutH3->addWidget(textThresholdInput);
    miscLayoutH3->addStretch(0);

    miscLayoutH4->addWidget(allowBeforeLabel);
    miscLayoutH4->addWidget(allowBeforeInput);
    miscLayoutH4->addWidget(allowAfterLabel);
    miscLayoutH4->addWidget(allowAfterInput);
    miscLayoutH4->addStretch(0);

    pline = new QFrame();
    pline->setFrameShape(QFrame::HLine);
    pline->setLineWidth(3);

    miscLayoutH5->addWidget(topoUrlLabel);
    miscLayoutH5->addWidget(topoUrlInput);

    miscLayoutH6->addWidget(landUseLabel);
    miscLayoutH6->addWidget(landUseInput);


    miscLayoutV1->addLayout(miscLayoutH1);
    miscLayoutV1->addLayout(miscLayoutH2);
    miscLayoutV1->addLayout(miscLayoutH3);
    miscLayoutV1->addLayout(miscLayoutH4);
    miscLayoutV1->addWidget(pline);
    miscLayoutV1->addLayout(miscLayoutH5);
    miscLayoutV1->addLayout(miscLayoutH6);
    setLayout(miscLayoutV1);
}

viewMiscConfigDialog::~viewMiscConfigDialog()
{
    delete miscLayoutH1;
}

#include "viewWindDialog.h"


viewWindDialog::viewWindDialog(QWidget *parent) : QDialog(parent)
{
    setWindowTitle("Winds Configuration");

    windSelectLabel = new QLabel;
    windSelectLabel->setText("Select Wind Layer:");

    windSelector = new QComboBox;
    windSelector->addItem("Surf");
    windSelector->addItem("RUC");
    windSelector->addItem("VDRAS");
    windSelector->addItem("DDOP");

    windUrlLabel = new QLabel;
    windUrlLabel->setText("Url:");
    windUrlInput = new QLineEdit;

    windColorSelectText = new QLabel;
    windColorSelectText->setText("Select Color:");
    windColorSelect = new QComboBox;
    windColorSelect->addItem("yellow");
    windColorSelect->addItem("cyan");
    windColorSelect->addItem("orange");
    windColorSelect->addItem("red");

    windNumLabel = new QLabel;
    windNumLabel->setText("Number:");
    windNumSlider = new QSlider(Qt::Horizontal, this);

    windWidthLabel = new QLabel;
    windWidthLabel->setText("Width:");
    windWidthSlider = new QSlider(Qt::Horizontal, this);

    windLengthLabel = new QLabel;
    windLengthLabel->setText("Length:");
    windLengthSlider = new QSlider(Qt::Horizontal, this);

    UNameLabel = new QLabel;
    UNameLabel->setText("UName:");
    UNameInput = new QLineEdit;
    UNameInput->setText("UWind");

    VNameLabel = new QLabel;
    VNameLabel->setText("VName:");
    VNameInput = new QLineEdit;
    VNameInput->setText("VWind");

    WNameLabel = new QLabel;
    WNameLabel->setText("WName:");
    WNameInput = new QLineEdit;
    WNameInput->setText("NA");

    windTimeSlopLabel = new QLabel;
    windTimeSlopLabel->setText("Time Slop:");
    windTimeSlopInput = new QLineEdit;
    windTimeSlopInput->setText("100v/t");

    windTimeOffsetLabel = new QLabel;
    windTimeOffsetLabel->setText("Time Offset:");
    windTimeOffsetInput = new QLineEdit;
    windTimeOffsetInput->setText("0v/t");

    windAltitudeOffsetLabel = new QLabel;
    windAltitudeOffsetLabel->setText("Altitude Offset:");
    windAltitudeOffsetInput = new QLineEdit;
    windAltitudeOffsetInput->setText("0v/t");

    windStylesLabel = new QLabel;
    windStylesLabel->setText("Style:");
    windStyles = new QComboBox;
    windStyles->addItem("Arrows");
    windStyles->addItem("Tufts");
    windStyles->addItem("Barbs");
    windStyles->addItem("Vectors");
    windStyles->addItem("T Vectors");
    windStyles->addItem("L Barbs");
    windStyles->addItem("Met Barbs");
    windStyles->addItem("SH Barbs");
    windStyles->addItem("LSH Barbs");

    windLegendLabel = new QLabel;
    windLegendLabel->setText("Show Legend:");
    windLegendSelect = new QCheckBox;

    windLayoutH1 = new QHBoxLayout;
    windLayoutH2 = new QHBoxLayout;
    windLayoutH3 = new QHBoxLayout;

    windLayoutV1 = new QVBoxLayout;
    windLayoutV2 = new QVBoxLayout;
    windLayoutV3 = new QVBoxLayout;
    windLayoutV4 = new QVBoxLayout;
    windLayoutVAll = new QVBoxLayout;


    windLayoutH1->addWidget(windSelectLabel);
    windLayoutH1->addWidget(windSelector);
    windLayoutH1->addStretch(0);

    windLayoutH2->addWidget(windUrlLabel);
    windLayoutH2->addWidget(windUrlInput);

    windLayoutV1->addWidget(windColorSelectText);
    windLayoutV1->addWidget(windColorSelect);
    windLayoutV1->addStretch(0);
    windLayoutV1->addWidget(windStylesLabel);
    windLayoutV1->addWidget(windStyles);
    windLayoutV1->addStretch(0);
    windLayoutV1->addWidget(windLegendLabel);
    windLayoutV1->addWidget(windLegendSelect);

    windLayoutV2->addWidget(windNumLabel);
    windLayoutV2->addWidget(windNumSlider);
    windLayoutV2->addSpacing(10);
    windLayoutV2->addStretch(0);
    windLayoutV2->addWidget(windWidthLabel);
    windLayoutV2->addWidget(windWidthSlider);
    windLayoutV2->addSpacing(10);
    windLayoutV2->addStretch(0);
    windLayoutV2->addWidget(windLengthLabel);
    windLayoutV2->addWidget(windLengthSlider);

    windLayoutV3->addWidget(UNameLabel);
    windLayoutV3->addWidget(UNameInput);
    windLayoutV3->addSpacing(10);
    windLayoutV3->addStretch(0);
    windLayoutV3->addWidget(VNameLabel);
    windLayoutV3->addWidget(VNameInput);
    windLayoutV3->addSpacing(10);
    windLayoutV3->addStretch(0);
    windLayoutV3->addWidget(WNameLabel);
    windLayoutV3->addWidget(WNameInput);

    windLayoutV4->addWidget(windTimeSlopLabel);
    windLayoutV4->addWidget(windTimeSlopInput);
    windLayoutV4->addSpacing(10);
    windLayoutV4->addStretch(0);
    windLayoutV4->addWidget(windTimeOffsetLabel);
    windLayoutV4->addWidget(windTimeOffsetInput);
    windLayoutV4->addSpacing(10);
    windLayoutV4->addStretch(0);
    windLayoutV4->addWidget(windAltitudeOffsetLabel);
    windLayoutV4->addWidget(windAltitudeOffsetInput);

    windLayoutH3->addLayout(windLayoutV1);
    windLayoutH3->addLayout(windLayoutV2);
    windLayoutH3->addLayout(windLayoutV3);
    windLayoutH3->addLayout(windLayoutV4);
    windLayoutH3->addStretch(0);

    windLayoutVAll->addLayout(windLayoutH1);
    windLayoutVAll->addLayout(windLayoutH2);
    windLayoutVAll->addLayout(windLayoutH3);

    setLayout(windLayoutVAll);
}


viewWindDialog::~viewWindDialog()
{
    delete windLayoutVAll;
    /*
    delete windSelectLabel;
    delete windUrlLabel;
    delete windColorSelectText;
    delete windNumLabel;
    delete windWidthLabel;
    delete windLengthLabel;
    delete UNameLabel;
    delete VNameLabel;
    delete WNameLabel;
    delete windTimeSlopLabel;
    delete windTimeOffsetLabel;
    delete windAltitudeOffsetLabel;
    delete windStylesLabel;
    delete windLegendLabel;
    delete windSelector;
    delete windColorSelect;
    delete windStyles;
    delete windUrlInput;
    delete UNameInput;
    delete VNameInput;
    delete WNameInput;
    delete windTimeSlopInput;
    delete windTimeOffsetInput;
    delete windAltitudeOffsetInput;
    delete windNumSlider;
    delete windWidthSlider;
    delete windLengthSlider;
    delete windLegendSelect;
    delete windLayoutH1;
    delete windLayoutH2;
    delete windLayoutH3;
    delete windLayoutV1;
    delete windLayoutV2;
    delete windLayoutV3;
    delete windLayoutV4;
    */
}






































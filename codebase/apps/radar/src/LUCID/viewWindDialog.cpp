#include "viewWindDialog.h"

//This file controls how the wind configurations dialog window looks for the user.
//it should work in tandem with contWindDialog

viewWindDialog::viewWindDialog(QWidget *parent) : QDialog(parent)
{
    //this class sets up the wind indicators Configuration window
    setWindowTitle("Winds Configuration");

    //wind layer selector label and combobox with dummy options
    windSelectLabel = new QLabel;
    windSelectLabel->setText("Select Wind Layer:");
    windSelector = new QComboBox;

    //Url label and line edit with no conneciton yet
    windUrlLabel = new QLabel;
    windUrlLabel->setText("Url:");
    windUrlInput = new QLineEdit;

    //color selector label and combobox with dummy options
    windColorSelectText = new QLabel;
    windColorSelectText->setText("Select Color:");
    windColorSelect = new QComboBox;

    //number label and slider with no connection yet
    windNumLabel = new QLabel;
    windNumLabel->setText("Number:");
    windNumSlider = new QSlider(Qt::Horizontal, this);

    //width label and slider with no connection yet
    windWidthLabel = new QLabel;
    windWidthLabel->setText("Width:");
    windWidthSlider = new QSlider(Qt::Horizontal, this);

    //length label and slider with no connection yet
    windLengthLabel = new QLabel;
    windLengthLabel->setText("Length:");
    windLengthSlider = new QSlider(Qt::Horizontal, this);

    //Uname label and line edit with no connection yet
    UNameLabel = new QLabel;
    UNameLabel->setText("UName:");
    UNameInput = new QLineEdit;
    UNameInput->setText("UWind");

    //Vname label and line edit with no connection yet
    VNameLabel = new QLabel;
    VNameLabel->setText("VName:");
    VNameInput = new QLineEdit;
    VNameInput->setText("VWind");

    //Wname label and line edit with no connection yet
    WNameLabel = new QLabel;
    WNameLabel->setText("WName:");
    WNameInput = new QLineEdit;
    WNameInput->setText("NA");

    //time slop label and line edit with no connection yet
    windTimeSlopLabel = new QLabel;
    windTimeSlopLabel->setText("Time Slop:");
    windTimeSlopInput = new QLineEdit;
    windTimeSlopInput->setText("100v/t");

    //time offset label and line edit with no connection yet
    windTimeOffsetLabel = new QLabel;
    windTimeOffsetLabel->setText("Time Offset:");
    windTimeOffsetInput = new QLineEdit;
    windTimeOffsetInput->setText("0v/t");

    //altitude offset label and line edit with no connection yet
    windAltitudeOffsetLabel = new QLabel;
    windAltitudeOffsetLabel->setText("Altitude Offset:");
    windAltitudeOffsetInput = new QLineEdit;
    windAltitudeOffsetInput->setText("0v/t");

    //styles label and combo box with dummy options
    windStylesLabel = new QLabel;
    windStylesLabel->setText("Style:");
    windStyles = new QComboBox;

    //Show legend label and checkbox with no connection yet
    windLegendLabel = new QLabel;
    windLegendLabel->setText("Show Legend:");
    windLegendSelect = new QCheckBox;

    //layouts
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
    //as of now, all pointers go into windLayoutVAll, so that is all that needs to be deleted
    delete windLayoutVAll;
}






































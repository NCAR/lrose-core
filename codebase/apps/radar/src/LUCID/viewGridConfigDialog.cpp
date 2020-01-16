#include "viewGridConfigDialog.h"

//This file should control how data is displayed in the
//Grid Data Layers Configuration dialog box
//It should work hand in hand with contGridConfigDialog,
//which controls what information is displayed.

viewGridConfigDialog::viewGridConfigDialog(QWidget *parent) : QDialog(parent)
{
    //this class sets up the Grid Data Layers Configuration window
    setWindowTitle("Grid Data Layers Configuration");

    //Grid data layer label and combo box with dummy numbers
    gridLayerLabel = new QLabel;
    gridLayerSelector = new QComboBox;

    //Grid selector label and combo box with dummy names
    gridLabel = new QLabel;
    gridLabel->setText("Grid:");
    gridSelector = new QComboBox;

    //Url label and line edit with no conneciton yet
    gridUrlLabel = new QLabel;
    gridUrlLabel->setText("Url:");
    gridUrlInput = new QLineEdit;

    //layer level label and combo box with dummy options
    gridTopBotLabel = new QLabel;
    gridTopBotLabel->setText("Layers on:");
    gridTopBotSelector = new QComboBox;

    //Show legend label and checkbox with no connection yet
    gridLegendLabel = new QLabel;
    gridLegendLabel->setText("Show Legend:");
    gridLegendBox = new QCheckBox;

    //color min value label and line edit with no conneciton yet
    gridMinValueLabel = new QLabel;
    gridMinValueInput = new QLineEdit;
    gridMinValueLabel->setText("Colors Min Value:");

    //color min value label and line edit with no conneciton yet
    gridMaxValueLabel = new QLabel;
    gridMaxValueInput = new QLineEdit;
    gridMaxValueLabel->setText("Colors Max Value:");

    //Delta label and line edit with no conneciton yet
    gridDeltaLabel = new QLabel;
    gridDeltaLabel->setText("Delta:");
    gridDeltaInput = new QLineEdit;

    //Time slop label and line edit with no conneciton yet
    gridTimeSlopLabel = new QLabel;
    gridTimeSlopLabel->setText("Time Slop (min):");
    gridTimeSlopInput = new QLineEdit;

    //Time offset label and line edit with no conneciton yet
    gridTimeOffsetLabel = new QLabel;
    gridTimeOffsetLabel->setText("Time Offset (min);");
    gridTimeOffsetInput = new QLineEdit;

    //Altitude offset label and line edit with no conneciton yet
    gridAltOffsetLabel = new QLabel;
    gridAltOffsetLabel->setText("Altitude Offset:");
    gridAltOffsetInput = new QLineEdit;

    //color map button with no connection yet
    gridColorMap = new QPushButton;
    gridColorMap->setText("Color Map");

    //auto update label and checkbox with no connection yet
    gridAutoUpdateLabel = new QLabel;
    gridAutoUpdateLabel->setText("Auto Update:");
    gridAutoUpdateBox = new QCheckBox;

    //Request composite label and checkbox with no connection yet
    gridRequestCompositeLabel = new QLabel;
    gridRequestCompositeLabel->setText("Request Composite:");
    gridRequestCompositeBox = new QCheckBox;

    //Autoscale label and checkbox with no connection yet
    gridAutoscaleLabel = new QLabel;
    gridAutoscaleLabel->setText("Autoscale:");
    gridAutoscaleBox = new QCheckBox;

    //colorscale label representative, purpose unclear at moment
    gridColorScale = new QLabel;
    gridColorScale->setText("Colorscale: dbz.colors (editable?)");

    //Units label representative, purpose unclear at moment
    gridUnits = new QLabel;
    gridUnits->setText("Units: dBz (editable?)");

    //Render as: label and combo box with dummy options
    gridRenderAsLabel = new QLabel;
    gridRenderAsLabel->setText("Render as:");
    gridRenderAsSelector = new QComboBox;

    //layout pointer initiations
    gridWindowLayoutH1 = new QHBoxLayout;
    gridWindowLayoutH2 = new QHBoxLayout;
    gridWindowLayoutH3 = new QHBoxLayout;
    gridWindowLayoutH4 = new QHBoxLayout;
    gridWindowLayoutH5 = new QHBoxLayout;
    gridWindowLayoutH6 = new QHBoxLayout;
    gridWindowLayoutH7 = new QHBoxLayout;
    gridWindowLayoutH8 = new QHBoxLayout;
    gridWindowLayoutH9 = new QHBoxLayout;
    gridWindowLayoutH10 = new QHBoxLayout;
    gridWindowLayoutH11 = new QHBoxLayout;
    gridWindowLayoutH12 = new QHBoxLayout;
    gridWindowLayoutH13 = new QHBoxLayout;
    gridWindowLayoutH14 = new QHBoxLayout;
    gridWindowLayoutH15 = new QHBoxLayout;
    gridWindowLayoutH16 = new QHBoxLayout;
    gridWindowLayoutV1 = new QVBoxLayout;
    gridWindowLayoutV2 = new QVBoxLayout;
    gridWindowLayoutVAll = new QVBoxLayout;

    //adding widgets to layouts
    gridWindowLayoutH1->addWidget(gridLayerLabel);
    gridWindowLayoutH1->addWidget(gridLayerSelector);
    gridWindowLayoutH1->addWidget(gridLabel);
    gridWindowLayoutH1->addWidget(gridSelector);
    gridWindowLayoutH1->addStretch(0);

    gridWindowLayoutH2->addWidget(gridUrlLabel);
    gridWindowLayoutH2->addWidget(gridUrlInput);

    gridWindowLayoutH3->addWidget(gridUnits);
    gridWindowLayoutH3->addWidget(gridColorMap);
    gridWindowLayoutH3->addWidget(gridColorScale);

    gridWindowLayoutH4->addWidget(gridLegendLabel);
    gridWindowLayoutH4->addWidget(gridLegendBox);
    gridWindowLayoutH4->addStretch(0);
    gridWindowLayoutH5->addWidget(gridAutoUpdateLabel);
    gridWindowLayoutH5->addWidget(gridAutoUpdateBox);
    gridWindowLayoutH5->addStretch(0);
    gridWindowLayoutH6->addWidget(gridRequestCompositeLabel);
    gridWindowLayoutH6->addWidget(gridRequestCompositeBox);
    gridWindowLayoutH6->addStretch(0);
    gridWindowLayoutH7->addWidget(gridAutoscaleLabel);
    gridWindowLayoutH7->addWidget(gridAutoscaleBox);
    gridWindowLayoutH7->addStretch(0);
    gridWindowLayoutH8->addWidget(gridTopBotLabel);
    gridWindowLayoutH8->addWidget(gridTopBotSelector);
    gridWindowLayoutH8->addStretch(0);
    gridWindowLayoutH9->addWidget(gridRenderAsLabel);
    gridWindowLayoutH9->addWidget(gridRenderAsSelector);
    gridWindowLayoutH9->addStretch(0);
    gridWindowLayoutV1->addLayout(gridWindowLayoutH4);
    gridWindowLayoutV1->addLayout(gridWindowLayoutH5);
    gridWindowLayoutV1->addLayout(gridWindowLayoutH6);
    gridWindowLayoutV1->addLayout(gridWindowLayoutH7);
    gridWindowLayoutV1->addLayout(gridWindowLayoutH8);
    gridWindowLayoutV1->addLayout(gridWindowLayoutH9);

    gridWindowLayoutH10->addWidget(gridMinValueLabel);
    gridWindowLayoutH10->addStretch(0);
    gridWindowLayoutH10->addWidget(gridMinValueInput);
    gridWindowLayoutH11->addWidget(gridMaxValueLabel);
    gridWindowLayoutH11->addStretch(0);
    gridWindowLayoutH11->addWidget(gridMaxValueInput);
    gridWindowLayoutH12->addWidget(gridDeltaLabel);
    gridWindowLayoutH12->addStretch(0);
    gridWindowLayoutH12->addWidget(gridDeltaInput);
    gridWindowLayoutH13->addWidget(gridTimeSlopLabel);
    gridWindowLayoutH13->addStretch(0);
    gridWindowLayoutH13->addWidget(gridTimeSlopInput);
    gridWindowLayoutH14->addWidget(gridTimeOffsetLabel);
    gridWindowLayoutH14->addStretch(0);
    gridWindowLayoutH14->addWidget(gridTimeOffsetInput);
    gridWindowLayoutH15->addWidget(gridAltOffsetLabel);
    gridWindowLayoutH15->addStretch(0);
    gridWindowLayoutH15->addWidget(gridAltOffsetInput);
    gridWindowLayoutV2->addLayout(gridWindowLayoutH10);
    gridWindowLayoutV2->addLayout(gridWindowLayoutH11);
    gridWindowLayoutV2->addLayout(gridWindowLayoutH12);
    gridWindowLayoutV2->addLayout(gridWindowLayoutH13);
    gridWindowLayoutV2->addLayout(gridWindowLayoutH14);
    gridWindowLayoutV2->addLayout(gridWindowLayoutH15);

    gridWindowLayoutH16->addLayout(gridWindowLayoutV1);
    gridWindowLayoutH16->addLayout(gridWindowLayoutV2);

    gridWindowLayoutVAll->addLayout(gridWindowLayoutH1);
    gridWindowLayoutVAll->addLayout(gridWindowLayoutH2);
    gridWindowLayoutVAll->addLayout(gridWindowLayoutH3);
    gridWindowLayoutVAll->addLayout(gridWindowLayoutH16);

    setLayout(gridWindowLayoutVAll);
}

viewGridConfigDialog::~viewGridConfigDialog()
{
    //as of now, all pointers go into gridWindowLayoutVAll, so that is all that needs to be deleted
    delete gridWindowLayoutVAll;
}

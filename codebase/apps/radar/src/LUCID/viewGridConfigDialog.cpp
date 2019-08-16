#include "viewGridConfigDialog.h"

viewGridConfigDialog::viewGridConfigDialog(QWidget *parent) : QDialog(parent)
{
    setWindowTitle("Grid Data Layers Configuration");

    gridLayerLabel = new QLabel;
    gridLayerLabel->setText("Grid Data Layer:");
    gridLayerSelector = new QComboBox;
    for(int i=0; i<16; i++)
    {   gridLayerSelector->addItem(QString::number(i+1));   }

    gridLabel = new QLabel;
    gridLabel->setText("Grid:");
    gridSelector = new QComboBox;
    gridSelector->addItem("KFTG DBZ");
    gridSelector->addItem("=NEXRAD=");
    gridSelector->addItem("=NEXRAD=");
    gridSelector->addItem("=NEXRAD=");
    gridSelector->addItem("=NEXRAD=");
    gridSelector->addItem("MANY MORE");

    gridUrlLabel = new QLabel;
    gridUrlLabel->setText("Url:");
    gridUrlInput = new QLineEdit;

    gridTopBotLabel = new QLabel;
    gridTopBotLabel->setText("Layers on:");
    gridTopBotSelector = new QComboBox;
    gridTopBotSelector->addItem("Top");
    gridTopBotSelector->addItem("Bottom");

    gridLegendLabel = new QLabel;
    gridLegendLabel->setText("Show Legend:");
    gridLegendBox = new QCheckBox;


    gridMinValueLabel = new QLabel;
    gridMinValueLabel->setText("Colors Min Value:");
    gridMinValueInput = new QLineEdit;
    gridMinValueInput->setText("-30");

    gridMaxValueLabel = new QLabel;
    gridMaxValueLabel->setText("Colors Max Value:");
    gridMaxValueInput = new QLineEdit;
    gridMaxValueInput->setText("100");

    gridDeltaLabel = new QLabel;
    gridDeltaLabel->setText("Delta:");
    gridDeltaInput = new QLineEdit;

    gridTimeSlopLabel = new QLabel;
    gridTimeSlopLabel->setText("Time Slop (min):");
    gridTimeSlopInput = new QLineEdit;
    gridTimeSlopInput->setText("100");

    gridTimeOffsetLabel = new QLabel;
    gridTimeOffsetLabel->setText("Time Offset (min);");
    gridTimeOffsetInput = new QLineEdit;
    gridTimeOffsetInput->setText("0");

    gridAltOffsetLabel = new QLabel;
    gridAltOffsetLabel->setText("Altitude Offset:");
    gridAltOffsetInput = new QLineEdit;
    gridAltOffsetInput->setText("0");

    gridColorMap = new QPushButton;
    gridColorMap->setText("Color Map");

    gridAutoUpdateLabel = new QLabel;
    gridAutoUpdateLabel->setText("Auto Update:");
    gridAutoUpdateBox = new QCheckBox;

    gridRequestCompositeLabel = new QLabel;
    gridRequestCompositeLabel->setText("Request Composite:");
    gridRequestCompositeBox = new QCheckBox;

    gridAutoscaleLabel = new QLabel;
    gridAutoscaleLabel->setText("Autoscale:");
    gridAutoscaleBox = new QCheckBox;

    gridColorScale = new QLabel;
    gridColorScale->setText("Colorscale: dbz.colors (editable?)");

    gridUnits = new QLabel;
    gridUnits->setText("Units: dBz (editable?)");

    gridRenderAsLabel = new QLabel;
    gridRenderAsLabel->setText("Render as:");
    gridRenderAsSelector = new QComboBox;
    gridRenderAsSelector->addItem("Polygons");
    gridRenderAsSelector->addItem("Contours");
    gridRenderAsSelector->addItem("Auto Contours");
    gridRenderAsSelector->addItem("Line Contours");



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

}

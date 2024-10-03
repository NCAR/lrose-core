#include "viewOverlaysDialog.h"


//This file will control how info is diplayed in the overlays dialog window.
//It should work in tandem with contOverlaysDialog

viewOverlaysDialog::viewOverlaysDialog(QWidget *parent) : QDialog(parent)
{
    setWindowTitle("Overlays");


    //contour layers
    lineLayerLabel = new QLabel;
    lineLayerLabel->setText("LINE CONTOUR LAYER");
    lineLayerSelector = new QComboBox;

    activeLabel = new QLabel;
    activeLabel->setText("Active:");
    activeCheckBox = new QCheckBox;

    fieldLayerSelector = new QComboBox;

    fromLabel = new QLabel;
    fromLabel->setText("From:");
    fromInput = new QLineEdit;

    toLabel = new QLabel;
    toLabel->setText("To:");
    toInput = new QLineEdit;

    byLabel = new QLabel;
    byLabel->setText("By:");
    byInput = new QLineEdit;

    showLegendLabel = new QLabel;
    showLegendLabel->setText("Show Legend:");
    showLegendCheck = new QCheckBox;

    colorSelector = new QComboBox;

    lineLabelsLabel = new QLabel;
    lineLabelsLabel->setText("Line Labels:");
    lineLabelsCheckBox = new QCheckBox;

    labelSizeLabel = new QLabel;
    labelSizeLabel->setText("Label Size:");
    labelSizeSlider = new QSlider(Qt::Horizontal);


    //map overlays
    mapsLabel = new QLabel;
    mapsLabel->setText("MAP OVERLAYS");

    changeColorButton = new QPushButton;
    changeColorButton->setText("Change Color");

    mapsList = new QListWidget;

    rangeRingLabel = new QLabel;
    rangeRingLabel->setText("Range Ring:");
    rangeRingCheckBox = new QCheckBox;

    rangeRingColors = new QPushButton;
    rangeRingColors->setText("Range Ring Color");

    landUseLabel = new QLabel;
    landUseLabel->setText("Land Use:");
    landUseCheckBox = new QCheckBox;

    brightnessLabel = new QLabel;
    brightnessLabel->setText("Brightness:");
    brightnessSlider = new QSlider(Qt::Horizontal);

    terrainMaskingLabel = new QLabel;
    terrainMaskingLabel->setText("Terrain Masking:");
    terrainMaskingCheck = new QCheckBox;

    savePlanViewButton = new QPushButton;
    savePlanViewButton->setText("Save Plan View Image to File...");


    line1 = new QFrame();
    line1->setFrameShape(QFrame::HLine);
    line1->setLineWidth(3);
    line2 = new QFrame();
    line2->setFrameShape(QFrame::HLine);
    line2->setLineWidth(3);
    line3 = new QFrame();
    line3->setFrameShape(QFrame::HLine);
    line3->setLineWidth(3);


    overLayoutH1 = new QHBoxLayout;
    overLayoutH2 = new QHBoxLayout;
    overLayoutH3 = new QHBoxLayout;
    overLayoutH4 = new QHBoxLayout;
    overLayoutH5 = new QHBoxLayout;
    overLayoutH6 = new QHBoxLayout;
    overLayoutH7 = new QHBoxLayout;
    overLayoutH8 = new QHBoxLayout;

    overLayoutV1 = new QVBoxLayout;


    overLayoutH1->addWidget(lineLayerLabel);
    overLayoutH1->addWidget(lineLayerSelector);
    overLayoutH1->addSpacing(20);
    overLayoutH1->addWidget(activeLabel);
    overLayoutH1->addWidget(activeCheckBox);
    overLayoutH1->addSpacing(20);
    overLayoutH1->addWidget(fieldLayerSelector);
    overLayoutH1->addStretch(0);

    overLayoutH2->addWidget(fromLabel);
    overLayoutH2->addWidget(fromInput);
    overLayoutH2->addWidget(toLabel);
    overLayoutH2->addWidget(toInput);
    overLayoutH2->addWidget(byLabel);
    overLayoutH2->addWidget(byInput);

    overLayoutH3->addWidget(showLegendLabel);
    overLayoutH3->addWidget(showLegendCheck);
    overLayoutH3->addWidget(colorSelector);
    overLayoutH3->addStretch(0);

    overLayoutH4->addWidget(lineLabelsLabel);
    overLayoutH4->addWidget(lineLabelsCheckBox);
    overLayoutH4->addSpacing(20);
    overLayoutH4->addWidget(labelSizeLabel);
    overLayoutH4->addWidget(labelSizeSlider);
    overLayoutH4->addStretch(0);

    overLayoutH5->addWidget(mapsLabel);
    overLayoutH5->addWidget(changeColorButton);
    overLayoutH5->addStretch(0);

    overLayoutH6->addWidget(rangeRingLabel);
    overLayoutH6->addWidget(rangeRingCheckBox);
    overLayoutH6->addSpacing(20);
    overLayoutH6->addWidget(rangeRingColors);
    overLayoutH6->addStretch(0);

    overLayoutH7->addWidget(landUseLabel);
    overLayoutH7->addWidget(landUseCheckBox);
    overLayoutH7->addSpacing(20);
    overLayoutH7->addWidget(brightnessLabel);
    overLayoutH7->addWidget(brightnessSlider);
    overLayoutH7->addStretch(0);

    overLayoutH8->addWidget(terrainMaskingLabel);
    overLayoutH8->addWidget(terrainMaskingCheck);
    overLayoutH8->addStretch(0);

    overLayoutV1->addWidget(line1);
    overLayoutV1->addLayout(overLayoutH1);
    overLayoutV1->addLayout(overLayoutH2);
    overLayoutV1->addLayout(overLayoutH3);
    overLayoutV1->addLayout(overLayoutH4);
    overLayoutV1->addWidget(line2);
    overLayoutV1->addLayout(overLayoutH5);
    overLayoutV1->addWidget(mapsList);
    overLayoutV1->addLayout(overLayoutH6);
    overLayoutV1->addLayout(overLayoutH7);
    overLayoutV1->addLayout(overLayoutH8);
    overLayoutV1->addWidget(line3);
    overLayoutV1->addWidget(savePlanViewButton);


    setLayout(overLayoutV1);
}

viewOverlaysDialog::~viewOverlaysDialog()
{
    delete overLayoutV1;
}

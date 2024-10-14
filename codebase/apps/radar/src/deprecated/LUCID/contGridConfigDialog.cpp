#include "contGridConfigDialog.h"

//This file should control what is displayed in the
//Grid Data Layers Configuration dialog box
//It should work hand in hand with viewGridConfigDialog,
//which controls how the information is displayed.

contGridConfigDialog::contGridConfigDialog()
{
    //this class sets up the Grid Data Layers Configuration window
    gridConfigViewer = new viewGridConfigDialog;

    //Grid data layer label and combo box with dummy numbers
    gridConfigViewer->gridLayerLabel->setText("Grid Data Layer:");
    for(int i=0; i<16; i++)
    {   gridConfigViewer->gridLayerSelector->addItem(QString::number(i+1));   }

    //Grid selector label and combo box with dummy names
    gridConfigViewer->gridSelector->addItem("KFTG DBZ");
    gridConfigViewer->gridSelector->addItem("=NEXRAD=");
    gridConfigViewer->gridSelector->addItem("=NEXRAD=");
    gridConfigViewer->gridSelector->addItem("=NEXRAD=");
    gridConfigViewer->gridSelector->addItem("=NEXRAD=");
    gridConfigViewer->gridSelector->addItem("MANY MORE");

    //layer level label and combo box with dummy options
    gridConfigViewer->gridTopBotSelector->addItem("Top");
    gridConfigViewer->gridTopBotSelector->addItem("Bottom");

    //color min value label and line edit with no conneciton yet
    gridConfigViewer->gridMinValueInput->setText("-30");

    //color min value label and line edit with no conneciton yet
    gridConfigViewer->gridMaxValueInput->setText("100");

    //Time slop label and line edit with no conneciton yet
    gridConfigViewer->gridTimeSlopInput->setText("100");

    //Time offset label and line edit with no conneciton yet
    gridConfigViewer->gridTimeOffsetInput->setText("0");

    //Altitude offset label and line edit with no conneciton yet
    gridConfigViewer->gridAltOffsetInput->setText("0");

    //Render as: label and combo box with dummy options
    gridConfigViewer->gridRenderAsSelector->addItem("Polygons");
    gridConfigViewer->gridRenderAsSelector->addItem("Contours");
    gridConfigViewer->gridRenderAsSelector->addItem("Auto Contours");
    gridConfigViewer->gridRenderAsSelector->addItem("Line Contours");
}























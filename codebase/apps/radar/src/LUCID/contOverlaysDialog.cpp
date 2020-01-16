#include "contOverlaysDialog.h"


//This file will control what info is diplayed in the overlays dialog window.
//It should work in tandem with viewOverlaysDialog

contOverlaysDialog::contOverlaysDialog()
{
    overlaysViewer = new viewOverlaysDialog;

    for(int i=0; i<16; i++)
    {
        overlaysViewer->lineLayerSelector->addItem(QString::number(i));
    }

    overlaysViewer->fieldLayerSelector->addItem("KFTG DBZ");
    overlaysViewer->fieldLayerSelector->addItem("=NEXRAD=");
    overlaysViewer->fieldLayerSelector->addItem("=NEXRAD=");
    overlaysViewer->fieldLayerSelector->addItem("=NEXRAD=");
    overlaysViewer->fieldLayerSelector->addItem("=NEXRAD=");
    overlaysViewer->fieldLayerSelector->addItem("MANY MORE");

    overlaysViewer->colorSelector->addItem("Black");
    overlaysViewer->colorSelector->addItem("White");
    overlaysViewer->colorSelector->addItem("Yellow");
    overlaysViewer->colorSelector->addItem("Many More");

    overlaysViewer->mapsList->addItems(QStringList() << "NCAR Pink");
    overlaysViewer->mapsList->addItems(QStringList() << "NCAR Red");
    overlaysViewer->mapsList->addItems(QStringList() << "NCAR Yellow");
    overlaysViewer->mapsList->addItems(QStringList() << "NCAR Green");
    overlaysViewer->mapsList->addItems(QStringList() << "NCAR Blue");
    overlaysViewer->mapsList->addItems(QStringList() << "MANY MORE");
}

contOverlaysDialog::~contOverlaysDialog()
{
    delete overlaysViewer;
}













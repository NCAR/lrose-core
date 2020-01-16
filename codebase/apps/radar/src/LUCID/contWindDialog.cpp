#include "contWindDialog.h"

//This file controls what info is displayed in the wind configurations dialog window.
//it should work in tandem with viewWindDialog
//for now, only dummy data is being displayed to imitate CIDD

contWindDialog::contWindDialog()
{
    windDialogViewer = new viewWindDialog;

    windDialogViewer->windSelector->addItem("Surf");
    windDialogViewer->windSelector->addItem("RUC");
    windDialogViewer->windSelector->addItem("VDRAS");
    windDialogViewer->windSelector->addItem("DDOP");

    windDialogViewer->windColorSelect->addItem("yellow");
    windDialogViewer->windColorSelect->addItem("cyan");
    windDialogViewer->windColorSelect->addItem("orange");
    windDialogViewer->windColorSelect->addItem("red");

    windDialogViewer->windStyles->addItem("Arrows");
    windDialogViewer->windStyles->addItem("Tufts");
    windDialogViewer->windStyles->addItem("Barbs");
    windDialogViewer->windStyles->addItem("Vectors");
    windDialogViewer->windStyles->addItem("T Vectors");
    windDialogViewer->windStyles->addItem("L Barbs");
    windDialogViewer->windStyles->addItem("Met Barbs");
    windDialogViewer->windStyles->addItem("SH Barbs");
    windDialogViewer->windStyles->addItem("LSH Barbs");
}

contWindDialog::~contWindDialog()
{
    delete windDialogViewer;
}









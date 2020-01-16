#include "viewZoomOptions.h"

//This file controls how the zoom options dialog window looks.
//it should work in tandem with contZoomOptions

viewZoomOptions::viewZoomOptions(QWidget *parent) : QDialog(parent)
{
    //Eventually this widget will display options for different zoom levels
    //it will also be neat to save zoom positions to easily go back and forth.
    //right now though, it is just dummy stuff
    setWindowTitle("Zoomz");
    zoom10 = new QPushButton;
    zoom10->setText("Zoom 10");
    zoom100 = new QPushButton;
    zoom100->setText("Zoom 100");
    zoom1000 = new QPushButton;
    zoom1000->setText("Zoom 1000");
    zoomSaved = new QPushButton;
    zoomSaved->setText("Saved Zoom 1");
    zoomSaved2 = new QPushButton;
    zoomSaved2->setText("Saved Zoom 2");
    zoomReset = new QPushButton;
    zoomReset->setText("Reset Zoom");

    zoomLayout = new QVBoxLayout;
    zoomLayout->addWidget(zoomReset);
    zoomLayout->addWidget(zoom10);
    zoomLayout->addWidget(zoom100);
    zoomLayout->addWidget(zoom1000);
    zoomLayout->addWidget(zoomSaved);
    zoomLayout->addWidget(zoomSaved2);

    setLayout(zoomLayout);
}

viewZoomOptions::~viewZoomOptions()
{
    //as of now, all pointers go into zoomLayout, so that is all that needs to be deleted
    delete zoomLayout;
}

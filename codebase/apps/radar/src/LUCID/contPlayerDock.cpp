#include "contPlayerDock.h"


//This file will control the movie player dock widget displayed at the bottom of the main window
//It should work in tandem with viewPlayerDock
//this is where the connections should be made, which for now are just for dummy interactions

contPlayerDock::contPlayerDock()
{
    playerDockViewer = new viewPlayerDock;
    //class that creates the movie player dock

    //time and date display with dummy representative values
    //will represent selected time for playback loop
    playerDockViewer->frameTime->setText("12:15:33");
    playerDockViewer->frameDate->setText("08/15/2019");

    //these two connects are for the slider and the number of frames, it was a toughie had to make function frameChanged()
    connect(playerDockViewer->posIndicator, SIGNAL(sliderReleased()), this, SLOT(frameChanged()));
    connect(playerDockViewer->numFramesInput, SIGNAL(returnPressed()), this, SLOT(frameChanged()));
}

//function is used in moviedock,
//it's for connectiong the slider, frame indicator, and number of frames input.
void contPlayerDock::frameChanged()
{
    QString num = playerDockViewer->numFramesInput->text();
    float numF = num.toFloat();
    float pos = playerDockViewer->posIndicator->sliderPosition();
    int frame = int(ceil(pos*(numF/100)));
    playerDockViewer->frameIndicator->display(frame);
}

contPlayerDock::~contPlayerDock()
{
    delete playerDockViewer;
}







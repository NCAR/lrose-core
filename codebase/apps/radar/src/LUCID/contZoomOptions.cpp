#include "contZoomOptions.h"


//This file will control the zoom options dialog window.
//it should work in tandem with viewZoomOptions

contZoomOptions::contZoomOptions()
{
    zoomOptionsViewer = new viewZoomOptions;
}

contZoomOptions::~contZoomOptions()
{
    delete zoomOptionsViewer;
}

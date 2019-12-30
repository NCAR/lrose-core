#include "contZoomOptions.h"

contZoomOptions::contZoomOptions()
{
    zoomOptionsViewer = new viewZoomOptions;
}

contZoomOptions::~contZoomOptions()
{
    delete zoomOptionsViewer;
}

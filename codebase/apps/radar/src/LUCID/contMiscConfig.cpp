#include "contMiscConfig.h"

//This file will eventually control what info is diplayed in the miscellaneous
//configurations dialog window. It should work in tandem with viewMiscConfigDialog

contMiscConfig::contMiscConfig()
{
    miscConfigViewer = new viewMiscConfigDialog;
}
contMiscConfig::~contMiscConfig()
{
    delete miscConfigViewer;
}

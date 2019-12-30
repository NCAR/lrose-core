#include "contMiscConfig.h"

contMiscConfig::contMiscConfig()
{
    miscConfigViewer = new viewMiscConfigDialog;
}
contMiscConfig::~contMiscConfig()
{
    delete miscConfigViewer;
}

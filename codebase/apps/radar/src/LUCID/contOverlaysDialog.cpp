#include "contOverlaysDialog.h"

contOverlaysDialog::contOverlaysDialog()
{
    overlaysViewer = new viewOverlaysDialog;
}

contOverlaysDialog::~contOverlaysDialog()
{
    delete overlaysViewer;
}

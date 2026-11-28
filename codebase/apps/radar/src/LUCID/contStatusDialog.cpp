#include "contStatusDialog.h"


//This file will control what info is displayed in the status window.
//It should work in tandem with viewStatusDialog
//for now, only dummy info is displayed.

contStatusDialog::contStatusDialog()
{
    statusDialogViewer = new viewStatusDialog;

    //make status window list widget entries
    for(int i=0; i<40; i++)
    {
        statusDialogViewer->statuses->addItems(QStringList() << "Something is happening with the weather");
        statusDialogViewer->statuses->addItems(QStringList() << "Or maybe something is happening with CIDD");
        statusDialogViewer->statuses->addItems(QStringList() << "Hopefully this becomes something");
        statusDialogViewer->statuses->addItems(QStringList() << "Will the status of LUCID's progress be included");
        statusDialogViewer->statuses->addItems(QStringList() << "   in this status window? Doubt");
    }

    connect(statusDialogViewer->clear, SIGNAL(clicked()), statusDialogViewer->statuses, SLOT(clear()));
    connect(statusDialogViewer->close, SIGNAL(clicked()), statusDialogViewer, SLOT(reject()));
}

contStatusDialog::~contStatusDialog()
{
    delete statusDialogViewer;
}

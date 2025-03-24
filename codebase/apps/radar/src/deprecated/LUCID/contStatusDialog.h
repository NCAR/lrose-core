#ifndef CONTSTATUSDIALOG_H
#define CONTSTATUSDIALOG_H

#include <QObject>

#include "viewStatusDialog.h"

class contStatusDialog : public QObject
{
    Q_OBJECT
public:
    contStatusDialog();
    ~contStatusDialog();

    viewStatusDialog *statusDialogViewer;
};

#endif // CONTSTATUSDIALOG_H

#ifndef CONTOVERLAYSDIALOG_H
#define CONTOVERLAYSDIALOG_H

#include <QObject>

#include "viewOverlaysDialog.h"


class contOverlaysDialog : public QObject
{
    Q_OBJECT
public:
    contOverlaysDialog();
    ~contOverlaysDialog();

    viewOverlaysDialog *overlaysViewer;
};

#endif // CONTOVERLAYSDIALOG_H











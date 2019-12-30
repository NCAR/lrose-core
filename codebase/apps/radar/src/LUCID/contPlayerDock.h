#ifndef CONTPLAYERDOCK_H
#define CONTPLAYERDOCK_H

#include <cmath>
#include <QObject>

#include "viewPlayerDock.h"

class contPlayerDock : public QObject
{
    Q_OBJECT
public:
    contPlayerDock();
    ~contPlayerDock();
    viewPlayerDock *playerDockViewer;

private slots:
    void frameChanged();
};

#endif // CONTPLAYERDOCK_H

#ifndef CONTVSECTION_H
#define CONTVSECTION_H

#include <QObject>

#include "viewVsection.h"

class contVsection : public QObject
{
    Q_OBJECT
public:
    contVsection();
    ~contVsection();

    viewVsection *VsectionViewer;
    QLineSeries *series, *series1, *series2, *series3;
private:

};

#endif // CONTVSECTION_H

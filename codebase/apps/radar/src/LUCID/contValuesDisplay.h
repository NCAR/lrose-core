#ifndef CONTVALUESDISPLAY_H
#define CONTVALUESDISPLAY_H

#include <QMouseEvent>
#include <QToolTip>

#include "viewValuesDisplay.h"

class contValuesDisplay
{
public:
    contValuesDisplay();
    ~contValuesDisplay();
    void updateValues(QMouseEvent* event, QPoint p);
    viewValuesDisplay *valuesDisplayViewer;

private:

};

#endif // CONTVALUESDISPLAY_H

#include "contValuesDisplay.h"

//This file controls the values displayed in the values dialog window
//It should work in tandem with viewValuesDisplay
//For now, it is only displaying the coordinates of the cursor
//In the future, the coordinate values will need to be transformed to show the
//dBz values displayed.

contValuesDisplay::contValuesDisplay()
{
    valuesDisplayViewer = new viewValuesDisplay;
}

void contValuesDisplay::updateValues(QMouseEvent* event, QPoint p)
{
    valuesDisplayViewer->valueOf1->setText(QString::number( event->pos().x() ));
    valuesDisplayViewer->valueOf2->setText(QString::number( event->pos().y() ));
    valuesDisplayViewer->valueOf3->setText(QString::number( p.x() ));
    valuesDisplayViewer->valueOf4->setText(QString::number( p.y() ));
    if(p.x()>-1 && p.y()>-1)
    {
        QToolTip::showText(event->globalPos(),
                           //  In most scenarios you will have to change these for
                           //  the coordinate system you are working in.
                           QString::number( p.x() ) + ", " +
                           QString::number( p.y() ));
    }
}

contValuesDisplay::~contValuesDisplay()
{
    delete valuesDisplayViewer;
}









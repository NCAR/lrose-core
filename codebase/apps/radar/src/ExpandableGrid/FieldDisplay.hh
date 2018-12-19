#ifndef FIELDDISPLAY_H
#define FIELDDISPLAY_H


#include <QDialog>
#include <QWidget>

class QWidget;

class FieldDisplay 
{

public:
    FieldDisplay(QWidget *widget, unsigned int r, unsigned int c); // QWidget *parent = 0);
    ~FieldDisplay();
 
    QWidget *widget;
    unsigned int row;
    unsigned int col;
};

#endif 

#include "FieldDisplay.hh"


FieldDisplay::FieldDisplay(QWidget *widget, unsigned int r, unsigned int c)
{
  row = r;
  col = c;
  this->widget = widget;

}
FieldDisplay::~FieldDisplay() {}


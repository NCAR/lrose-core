#include "ClickableLabel.hh"

ClickableLabel::ClickableLabel(QWidget* parent, Qt::WindowFlags f)
    : QLabel(parent) {
  //_colorBar = NULL;
  setScaledContents(true);
}

ClickableLabel::~ClickableLabel() {}

void ClickableLabel::mousePressEvent(QMouseEvent* event) {
    emit clicked();
}


void ClickableLabel::mouseDoubleClickEvent(QMouseEvent *event) {
  emit doubleClicked(text());
}

/*
void ClickableLabel::setColorBar(ColorBar *colorBar) {
  _colorBar = colorBar;
}


void ClickableLabel::paintEvent(QPaintEvent *e) {
  //_colorBar->paintEvent(e);
  
  // -----
  // TODO: remember to free previous pixmap
  QPixmap *pixmap = colorBar->getPixmap();

  int w = width();
  int h = height();

  // -----
  clear();

  setPixmap(*pixmap);
}
*/

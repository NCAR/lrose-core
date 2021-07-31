#include "ClickableLabelNamed.hh"

ClickableLabelNamed::ClickableLabelNamed(QWidget* parent, Qt::WindowFlags f)
    : QLabel(parent) {
  setScaledContents(true);
}

ClickableLabelNamed::~ClickableLabelNamed() {}

void ClickableLabelNamed::mousePressEvent(QMouseEvent* event) {
    emit clicked(text());
}


void ClickableLabelNamed::mouseDoubleClickEvent(QMouseEvent *event) {
  emit doubleClicked(text());
}


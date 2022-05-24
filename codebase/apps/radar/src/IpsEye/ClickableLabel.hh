#ifndef CLICKABLELABEL_H
#define CLICKABLELABEL_H

#include "ColorBar.hh"
#include <QLabel>
#include <QWidget>
#include <Qt>

class ClickableLabel : public QLabel { 
    Q_OBJECT

public:
  explicit ClickableLabel(QWidget* parent = Q_NULLPTR, Qt::WindowFlags f = Qt::WindowFlags());
  virtual ~ClickableLabel();
  //  void setColorBar(ColorBar *colorBar);

signals:
  void clicked();

protected:
  void mousePressEvent(QMouseEvent* event);
  //  void paintEvent(QPaintEvent *e);


private:
  //  ColorBar *_colorBar;
};

#endif // CLICKABLELABEL_H

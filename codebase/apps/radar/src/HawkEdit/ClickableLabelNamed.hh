#ifndef CLICKABLELABELNAMED_H
#define CLICKABLELABELNAMED_H

#include <QLabel>
#include <QWidget>
#include <Qt>

class ClickableLabelNamed : public QLabel { 
    Q_OBJECT

public:
  explicit ClickableLabelNamed(QWidget* parent = Q_NULLPTR, Qt::WindowFlags f = Qt::WindowFlags());
  virtual ~ClickableLabelNamed();

signals:
  void clicked(QString text);
  void doubleClicked(QString text);

protected:
  void mousePressEvent(QMouseEvent* event);

  void mouseDoubleClickEvent(QMouseEvent *event);

private:
};

#endif // CLICKABLELABELNAMED_H

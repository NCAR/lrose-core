#include <QWidget>
#include <QPushButton>
#include <QFocusEvent>

#include "HiddenContextMenu.hh"

HiddenContextMenu::HiddenContextMenu()
{
  QPushButton(QString(">"));
/*
  connect(this, &HiddenContextMenu::focusInEvent, // (QFocusEvent *),
          this, &HiddenContextMenu::hoverOnEvent);
  connect(this, &HiddenContextMenu::focusOutEvent, // (QFocusEvent *),
          this, &HiddenContextMenu::hoverOffEvent);
*/
}
  
/*
void HiddenContextMenu::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == textEdit) {
        if (event->type() == QEvent::KeyPress) {
            QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
            qDebug() << "Ate key press" << keyEvent->key();
            setVisible(true);  
            return true;
        } else {
            return false;
        }
    } else {
        // pass the event on to the parent class
        return QMainWindow::eventFilter(obj, event);
    }
}
*/

/*
void HiddenContextMenu::hoverOffEvent()
{
  setVisible(false);  
}
*/



#ifndef YOURBUTTON_H
#define YOURBUTTON_H

#include <QWidget>
#include <QToolButton>
#include <QFocusEvent>
#include <QSize>

class YourButton : public QToolButton
{
    Q_OBJECT
    // proper constructor and other standard stuff 
    // ..
public:

    YourButton();

protected:
    void focusInEvent(QFocusEvent* e);
    void focusOutEvent(QFocusEvent* e);

public:
    QSize sizeHint() const {
        QSize result = QToolButton::sizeHint();
        if (hasFocus()) {
            result += QSize(20,20);
        }
        return result;
    }

};

#endif



/*

class MainWindow : public QMainWindow
{
public:
    MainWindow();

protected:
    bool eventFilter(QObject *obj, QEvent *ev) override;

private:
    QTextEdit *textEdit;
};

MainWindow::MainWindow()
{
    textEdit = new QTextEdit;
    setCentralWidget(textEdit);

    textEdit->installEventFilter(this);
}

bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == textEdit) {
        if (event->type() == QEvent::KeyPress) {
            QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
            qDebug() << "Ate key press" << keyEvent->key();
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

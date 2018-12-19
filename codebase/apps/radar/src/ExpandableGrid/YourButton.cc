#include <iostream>
#include <QWidget>
#include <QToolButton>

#include "YourButton.hh"

using namespace std; 

    // proper constructor and other standard stuff 
    // ..

    YourButton::YourButton() {
       // QToolButton("^");
    }

    void YourButton::focusInEvent(QFocusEvent* e) {
        cerr << " in focusInEvent " << endl;
        QToolButton::focusInEvent(e);
        updateGeometry();
    }

    void YourButton::focusOutEvent(QFocusEvent* e) {
        cerr << " in focusOutEvent " << endl;
        QToolButton::focusOutEvent(e);
        updateGeometry();
    }

/*
    QSize YourButton::sizeHint() const {
        QSize result = QToolButton::sizeHint();
        if (hasFocus()) {
            result += QSize(20,20);
        }
        return result;
    }
*/

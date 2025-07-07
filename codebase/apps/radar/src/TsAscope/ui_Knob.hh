/********************************************************************************
** Form generated from reading UI file 'Knob.ui'
**
** Created by: Qt User Interface Compiler version 4.8.7
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_KNOB_H
#define UI_KNOB_H

#include <QVariant>
#include <QAction>
#include <QApplication>
#include <QButtonGroup>
#include <QHeaderView>
#include <QLCDNumber>
#include <QLabel>
#include <QVBoxLayout>
#include <QWidget>
#include <qwt/qwt_knob.h>

QT_BEGIN_NAMESPACE

class Ui_Knob
{
public:
    QVBoxLayout *vboxLayout;
    QwtKnob *_knob;
    QVBoxLayout *vboxLayout1;
    QLCDNumber *_lcd;
    QLabel *_label;

    void setupUi(QWidget *Knob)
    {
        if (Knob->objectName().isEmpty())
            Knob->setObjectName(QString::fromUtf8("Knob"));
        Knob->resize(128, 179);
        QSizePolicy sizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(Knob->sizePolicy().hasHeightForWidth());
        Knob->setSizePolicy(sizePolicy);
        vboxLayout = new QVBoxLayout(Knob);
#ifndef Q_OS_MAC
        vboxLayout->setSpacing(6);
#endif
#ifndef Q_OS_MAC
        vboxLayout->setContentsMargins(9, 9, 9, 9);
#endif
        vboxLayout->setObjectName(QString::fromUtf8("vboxLayout"));
        _knob = new QwtKnob(Knob);
        _knob->setObjectName(QString::fromUtf8("_knob"));

        vboxLayout->addWidget(_knob);

        vboxLayout1 = new QVBoxLayout();
#ifndef Q_OS_MAC
        vboxLayout1->setSpacing(6);
#endif
        vboxLayout1->setContentsMargins(0, 0, 0, 0);
        vboxLayout1->setObjectName(QString::fromUtf8("vboxLayout1"));
        _lcd = new QLCDNumber(Knob);
        _lcd->setObjectName(QString::fromUtf8("_lcd"));
        _lcd->setSegmentStyle(QLCDNumber::Flat);

        vboxLayout1->addWidget(_lcd);

        _label = new QLabel(Knob);
        _label->setObjectName(QString::fromUtf8("_label"));
        sizePolicy.setHeightForWidth(_label->sizePolicy().hasHeightForWidth());
        _label->setSizePolicy(sizePolicy);
        _label->setAlignment(Qt::AlignCenter);

        vboxLayout1->addWidget(_label);


        vboxLayout->addLayout(vboxLayout1);


        retranslateUi(Knob);
        QObject::connect(_knob, SIGNAL(valueChanged(double)), _lcd, SLOT(display(double)));

        QMetaObject::connectSlotsByName(Knob);
    } // setupUi

    void retranslateUi(QWidget *Knob)
    {
      Knob->setWindowTitle(QApplication::translate("Knob", "Knob", 0));
      _label->setText(QApplication::translate("Knob", "text", 0));
    } // retranslateUi

};

namespace Ui {
    class Knob: public Ui_Knob {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_KNOB_H

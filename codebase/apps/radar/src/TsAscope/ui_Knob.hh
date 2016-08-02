// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
// ** Copyright UCAR (c) 1990 - 2016                                         
// ** University Corporation for Atmospheric Research (UCAR)                 
// ** National Center for Atmospheric Research (NCAR)                        
// ** Boulder, Colorado, USA                                                 
// ** BSD licence applies - redistribution and use in source and binary      
// ** forms, with or without modification, are permitted provided that       
// ** the following conditions are met:                                      
// ** 1) If the software is modified to produce derivative works,            
// ** such modified software should be clearly marked, so as not             
// ** to confuse it with the version available from UCAR.                    
// ** 2) Redistributions of source code must retain the above copyright      
// ** notice, this list of conditions and the following disclaimer.          
// ** 3) Redistributions in binary form must reproduce the above copyright   
// ** notice, this list of conditions and the following disclaimer in the    
// ** documentation and/or other materials provided with the distribution.   
// ** 4) Neither the name of UCAR nor the names of its contributors,         
// ** if any, may be used to endorse or promote products derived from        
// ** this software without specific prior written permission.               
// ** DISCLAIMER: THIS SOFTWARE IS PROVIDED "AS IS" AND WITHOUT ANY EXPRESS  
// ** OR IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED      
// ** WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.    
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
/********************************************************************************
** Form generated from reading UI file 'Knob.ui'
**
** Created: Tue Sep 27 06:30:15 2011
**      by: Qt User Interface Compiler version 4.6.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_KNOB_H
#define UI_KNOB_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QHeaderView>
#include <QtGui/QLCDNumber>
#include <QtGui/QLabel>
#include <QtGui/QVBoxLayout>
#include <QtGui/QWidget>
#include "qwt_knob.h"

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
        Knob->setWindowTitle(QApplication::translate("Knob", "Knob", 0, QApplication::UnicodeUTF8));
        _label->setText(QApplication::translate("Knob", "text", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class Knob: public Ui_Knob {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_KNOB_H

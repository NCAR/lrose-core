/********************************************************************************
** Form generated from reading UI file 'ScopePlot.ui'
**
** Created by: Qt User Interface Compiler version 4.8.7
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_SCOPEPLOT_H
#define UI_SCOPEPLOT_H

#include <QVariant>
#include <QAction>
#include <QApplication>
#include <QButtonGroup>
#include <QHeaderView>
#include <QVBoxLayout>
#include <QWidget>
#include <qwt/qwt_plot.h>

QT_BEGIN_NAMESPACE

class Ui_ScopePlot
{
public:
    QVBoxLayout *vboxLayout;
    QwtPlot *_qwtPlot;

    void setupUi(QWidget *ScopePlot)
    {
        if (ScopePlot->objectName().isEmpty())
            ScopePlot->setObjectName(QString::fromUtf8("ScopePlot"));
        ScopePlot->resize(558, 454);
        vboxLayout = new QVBoxLayout(ScopePlot);
#ifndef Q_OS_MAC
        vboxLayout->setSpacing(6);
#endif
#ifndef Q_OS_MAC
        vboxLayout->setContentsMargins(9, 9, 9, 9);
#endif
        vboxLayout->setObjectName(QString::fromUtf8("vboxLayout"));
        _qwtPlot = new QwtPlot(ScopePlot);
        _qwtPlot->setObjectName(QString::fromUtf8("_qwtPlot"));

        vboxLayout->addWidget(_qwtPlot);


        retranslateUi(ScopePlot);

        QMetaObject::connectSlotsByName(ScopePlot);
    } // setupUi

    void retranslateUi(QWidget *ScopePlot)
    {
        ScopePlot->setWindowTitle(QApplication::translate("ScopePlot", "ScopePlot", 0));
    } // retranslateUi

};

namespace Ui {
    class ScopePlot: public Ui_ScopePlot {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_SCOPEPLOT_H

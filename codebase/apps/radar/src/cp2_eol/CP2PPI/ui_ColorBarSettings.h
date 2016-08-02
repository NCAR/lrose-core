/********************************************************************************
** Form generated from reading ui file 'ColorBarSettings.ui'
**
** Created: Wed Aug 1 11:46:59 2007
**      by: Qt User Interface Compiler version 4.2.2
**
** WARNING! All changes made in this file will be lost when recompiling ui file!
********************************************************************************/

#ifndef UI_COLORBARSETTINGS_H
#define UI_COLORBARSETTINGS_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QComboBox>
#include <QtGui/QDialog>
#include <QtGui/QDialogButtonBox>
#include <QtGui/QDoubleSpinBox>
#include <QtGui/QFrame>
#include <QtGui/QHBoxLayout>
#include <QtGui/QLabel>
#include <QtGui/QVBoxLayout>

class Ui_ColorBarSettings
{
public:
    QVBoxLayout *vboxLayout;
    QLabel *label_3;
    QFrame *line;
    QComboBox *_mapComboBox;
    QHBoxLayout *hboxLayout;
    QDoubleSpinBox *_maxSpin;
    QLabel *label;
    QHBoxLayout *hboxLayout1;
    QDoubleSpinBox *_minSpin;
    QLabel *label_2;
    QDialogButtonBox *buttonBox;

    void setupUi(QDialog *ColorBarSettings)
    {
    ColorBarSettings->setObjectName(QString::fromUtf8("ColorBarSettings"));
    QSizePolicy sizePolicy(static_cast<QSizePolicy::Policy>(3), static_cast<QSizePolicy::Policy>(3));
    sizePolicy.setHorizontalStretch(0);
    sizePolicy.setVerticalStretch(0);
    sizePolicy.setHeightForWidth(ColorBarSettings->sizePolicy().hasHeightForWidth());
    ColorBarSettings->setSizePolicy(sizePolicy);
    vboxLayout = new QVBoxLayout(ColorBarSettings);
    vboxLayout->setSpacing(6);
    vboxLayout->setMargin(9);
    vboxLayout->setObjectName(QString::fromUtf8("vboxLayout"));
    label_3 = new QLabel(ColorBarSettings);
    label_3->setObjectName(QString::fromUtf8("label_3"));
    QSizePolicy sizePolicy1(static_cast<QSizePolicy::Policy>(3), static_cast<QSizePolicy::Policy>(3));
    sizePolicy1.setHorizontalStretch(0);
    sizePolicy1.setVerticalStretch(0);
    sizePolicy1.setHeightForWidth(label_3->sizePolicy().hasHeightForWidth());
    label_3->setSizePolicy(sizePolicy1);
    label_3->setAlignment(Qt::AlignCenter);

    vboxLayout->addWidget(label_3);

    line = new QFrame(ColorBarSettings);
    line->setObjectName(QString::fromUtf8("line"));
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);

    vboxLayout->addWidget(line);

    _mapComboBox = new QComboBox(ColorBarSettings);
    _mapComboBox->setObjectName(QString::fromUtf8("_mapComboBox"));

    vboxLayout->addWidget(_mapComboBox);

    hboxLayout = new QHBoxLayout();
    hboxLayout->setSpacing(6);
    hboxLayout->setMargin(0);
    hboxLayout->setObjectName(QString::fromUtf8("hboxLayout"));
    _maxSpin = new QDoubleSpinBox(ColorBarSettings);
    _maxSpin->setObjectName(QString::fromUtf8("_maxSpin"));

    hboxLayout->addWidget(_maxSpin);

    label = new QLabel(ColorBarSettings);
    label->setObjectName(QString::fromUtf8("label"));

    hboxLayout->addWidget(label);


    vboxLayout->addLayout(hboxLayout);

    hboxLayout1 = new QHBoxLayout();
    hboxLayout1->setSpacing(6);
    hboxLayout1->setMargin(0);
    hboxLayout1->setObjectName(QString::fromUtf8("hboxLayout1"));
    _minSpin = new QDoubleSpinBox(ColorBarSettings);
    _minSpin->setObjectName(QString::fromUtf8("_minSpin"));

    hboxLayout1->addWidget(_minSpin);

    label_2 = new QLabel(ColorBarSettings);
    label_2->setObjectName(QString::fromUtf8("label_2"));

    hboxLayout1->addWidget(label_2);


    vboxLayout->addLayout(hboxLayout1);

    buttonBox = new QDialogButtonBox(ColorBarSettings);
    buttonBox->setObjectName(QString::fromUtf8("buttonBox"));
    buttonBox->setOrientation(Qt::Horizontal);
    buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::NoButton|QDialogButtonBox::Ok);

    vboxLayout->addWidget(buttonBox);


    retranslateUi(ColorBarSettings);

    QSize size(168, 142);
    size = size.expandedTo(ColorBarSettings->minimumSizeHint());
    ColorBarSettings->resize(size);

    QObject::connect(buttonBox, SIGNAL(accepted()), ColorBarSettings, SLOT(accept()));
    QObject::connect(buttonBox, SIGNAL(rejected()), ColorBarSettings, SLOT(reject()));

    QMetaObject::connectSlotsByName(ColorBarSettings);
    } // setupUi

    void retranslateUi(QDialog *ColorBarSettings)
    {
    ColorBarSettings->setWindowTitle(QApplication::translate("ColorBarSettings", "Color Bar Settings", 0, QApplication::UnicodeUTF8));
    label_3->setText(QApplication::translate("ColorBarSettings", "Color Map Configuration", 0, QApplication::UnicodeUTF8));
    label->setText(QApplication::translate("ColorBarSettings", "Maximum", 0, QApplication::UnicodeUTF8));
    label_2->setText(QApplication::translate("ColorBarSettings", "Minimum", 0, QApplication::UnicodeUTF8));
    Q_UNUSED(ColorBarSettings);
    } // retranslateUi

};

namespace Ui {
    class ColorBarSettings: public Ui_ColorBarSettings {};
} // namespace Ui

#endif // UI_COLORBARSETTINGS_H

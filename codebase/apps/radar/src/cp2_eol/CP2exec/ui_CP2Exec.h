/********************************************************************************
** Form generated from reading ui file 'CP2Exec.ui'
**
** Created: Wed Aug 1 11:46:59 2007
**      by: Qt User Interface Compiler version 4.2.2
**
** WARNING! All changes made in this file will be lost when recompiling ui file!
********************************************************************************/

#ifndef UI_CP2EXEC_H
#define UI_CP2EXEC_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QDialog>
#include <QtGui/QDialogButtonBox>
#include <QtGui/QGridLayout>
#include <QtGui/QGroupBox>
#include <QtGui/QHBoxLayout>
#include <QtGui/QLabel>
#include <QtGui/QSpacerItem>
#include <QtGui/QVBoxLayout>

class Ui_CP2Exec
{
public:
    QVBoxLayout *vboxLayout;
    QGroupBox *groupBox_2;
    QHBoxLayout *hboxLayout;
    QLabel *_statusText;
    QHBoxLayout *hboxLayout1;
    QLabel *label;
    QLabel *_elapsedText;
    QGroupBox *groupBox_5;
    QHBoxLayout *hboxLayout2;
    QHBoxLayout *hboxLayout3;
    QLabel *label_2;
    QLabel *_gatesText;
    QHBoxLayout *hboxLayout4;
    QLabel *label_4;
    QLabel *_prfHzText;
    QHBoxLayout *hboxLayout5;
    QLabel *label_6;
    QLabel *_pwText;
    QHBoxLayout *hboxLayout6;
    QLabel *label_8;
    QLabel *_simAnglesText;
    QHBoxLayout *hboxLayout7;
    QLabel *label_9;
    QLabel *_prfSourceText;
    QGroupBox *groupBox;
    QGridLayout *gridLayout;
    QLabel *textLabel1_5_3;
    QLabel *textLabel1_5_2;
    QLabel *_chan2pulseCount;
    QLabel *_chan1pulseRate;
    QLabel *_chan0led;
    QLabel *textLabel3;
    QLabel *_chan2pulseRate;
    QLabel *textLabel1_5;
    QLabel *textLabel3_3;
    QLabel *textLabel3_4;
    QLabel *_chan0pulseCount;
    QLabel *textLabel4;
    QLabel *textLabel3_2;
    QLabel *_chan2led;
    QLabel *_chan1led;
    QLabel *_chan2errors;
    QLabel *_chan0errors;
    QLabel *_chan1pulseCount;
    QLabel *_chan1errors;
    QLabel *_chan0pulseRate;
    QGroupBox *groupBox_3;
    QGridLayout *gridLayout1;
    QLabel *textLabel3_6;
    QLabel *_chan2Volume;
    QLabel *_chan0Volume;
    QLabel *_chan1Volume;
    QLabel *_chan2Sweep;
    QLabel *_chan1Sweep;
    QLabel *_chan0Sweep;
    QLabel *_chan2El;
    QLabel *_chan1El;
    QLabel *_chan0El;
    QLabel *_chan0Az;
    QLabel *_chan1Az;
    QLabel *_chan2Az;
    QLabel *textLabel3_7;
    QLabel *textLabel3_5;
    QLabel *textLabel3_8;
    QLabel *textLabel1_5_5;
    QLabel *textLabel1_5_4;
    QLabel *textLabel1_6;
    QLabel *textLabel4_2;
    QGroupBox *groupBox_4;
    QHBoxLayout *hboxLayout8;
    QLabel *_networkIP;
    QLabel *_networkPort;
    QSpacerItem *spacerItem;
    QDialogButtonBox *buttonBox;

    void setupUi(QDialog *CP2Exec)
    {
    CP2Exec->setObjectName(QString::fromUtf8("CP2Exec"));
    QSizePolicy sizePolicy(static_cast<QSizePolicy::Policy>(1), static_cast<QSizePolicy::Policy>(1));
    sizePolicy.setHorizontalStretch(0);
    sizePolicy.setVerticalStretch(0);
    sizePolicy.setHeightForWidth(CP2Exec->sizePolicy().hasHeightForWidth());
    CP2Exec->setSizePolicy(sizePolicy);
    vboxLayout = new QVBoxLayout(CP2Exec);
    vboxLayout->setSpacing(6);
    vboxLayout->setMargin(9);
    vboxLayout->setObjectName(QString::fromUtf8("vboxLayout"));
    groupBox_2 = new QGroupBox(CP2Exec);
    groupBox_2->setObjectName(QString::fromUtf8("groupBox_2"));
    QSizePolicy sizePolicy1(static_cast<QSizePolicy::Policy>(3), static_cast<QSizePolicy::Policy>(3));
    sizePolicy1.setHorizontalStretch(0);
    sizePolicy1.setVerticalStretch(0);
    sizePolicy1.setHeightForWidth(groupBox_2->sizePolicy().hasHeightForWidth());
    groupBox_2->setSizePolicy(sizePolicy1);
    hboxLayout = new QHBoxLayout(groupBox_2);
    hboxLayout->setSpacing(6);
    hboxLayout->setMargin(9);
    hboxLayout->setObjectName(QString::fromUtf8("hboxLayout"));
    _statusText = new QLabel(groupBox_2);
    _statusText->setObjectName(QString::fromUtf8("_statusText"));
    QSizePolicy sizePolicy2(static_cast<QSizePolicy::Policy>(3), static_cast<QSizePolicy::Policy>(3));
    sizePolicy2.setHorizontalStretch(0);
    sizePolicy2.setVerticalStretch(0);
    sizePolicy2.setHeightForWidth(_statusText->sizePolicy().hasHeightForWidth());
    _statusText->setSizePolicy(sizePolicy2);
    _statusText->setWordWrap(false);

    hboxLayout->addWidget(_statusText);

    hboxLayout1 = new QHBoxLayout();
    hboxLayout1->setSpacing(6);
    hboxLayout1->setMargin(0);
    hboxLayout1->setObjectName(QString::fromUtf8("hboxLayout1"));
    label = new QLabel(groupBox_2);
    label->setObjectName(QString::fromUtf8("label"));
    label->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

    hboxLayout1->addWidget(label);

    _elapsedText = new QLabel(groupBox_2);
    _elapsedText->setObjectName(QString::fromUtf8("_elapsedText"));

    hboxLayout1->addWidget(_elapsedText);


    hboxLayout->addLayout(hboxLayout1);


    vboxLayout->addWidget(groupBox_2);

    groupBox_5 = new QGroupBox(CP2Exec);
    groupBox_5->setObjectName(QString::fromUtf8("groupBox_5"));
    hboxLayout2 = new QHBoxLayout(groupBox_5);
    hboxLayout2->setSpacing(6);
    hboxLayout2->setMargin(9);
    hboxLayout2->setObjectName(QString::fromUtf8("hboxLayout2"));
    hboxLayout3 = new QHBoxLayout();
    hboxLayout3->setSpacing(6);
    hboxLayout3->setMargin(0);
    hboxLayout3->setObjectName(QString::fromUtf8("hboxLayout3"));
    label_2 = new QLabel(groupBox_5);
    label_2->setObjectName(QString::fromUtf8("label_2"));
    label_2->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

    hboxLayout3->addWidget(label_2);

    _gatesText = new QLabel(groupBox_5);
    _gatesText->setObjectName(QString::fromUtf8("_gatesText"));

    hboxLayout3->addWidget(_gatesText);


    hboxLayout2->addLayout(hboxLayout3);

    hboxLayout4 = new QHBoxLayout();
    hboxLayout4->setSpacing(6);
    hboxLayout4->setMargin(0);
    hboxLayout4->setObjectName(QString::fromUtf8("hboxLayout4"));
    label_4 = new QLabel(groupBox_5);
    label_4->setObjectName(QString::fromUtf8("label_4"));
    label_4->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

    hboxLayout4->addWidget(label_4);

    _prfHzText = new QLabel(groupBox_5);
    _prfHzText->setObjectName(QString::fromUtf8("_prfHzText"));

    hboxLayout4->addWidget(_prfHzText);


    hboxLayout2->addLayout(hboxLayout4);

    hboxLayout5 = new QHBoxLayout();
    hboxLayout5->setSpacing(6);
    hboxLayout5->setMargin(0);
    hboxLayout5->setObjectName(QString::fromUtf8("hboxLayout5"));
    label_6 = new QLabel(groupBox_5);
    label_6->setObjectName(QString::fromUtf8("label_6"));
    label_6->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

    hboxLayout5->addWidget(label_6);

    _pwText = new QLabel(groupBox_5);
    _pwText->setObjectName(QString::fromUtf8("_pwText"));

    hboxLayout5->addWidget(_pwText);


    hboxLayout2->addLayout(hboxLayout5);

    hboxLayout6 = new QHBoxLayout();
    hboxLayout6->setSpacing(6);
    hboxLayout6->setMargin(0);
    hboxLayout6->setObjectName(QString::fromUtf8("hboxLayout6"));
    label_8 = new QLabel(groupBox_5);
    label_8->setObjectName(QString::fromUtf8("label_8"));
    label_8->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

    hboxLayout6->addWidget(label_8);

    _simAnglesText = new QLabel(groupBox_5);
    _simAnglesText->setObjectName(QString::fromUtf8("_simAnglesText"));

    hboxLayout6->addWidget(_simAnglesText);


    hboxLayout2->addLayout(hboxLayout6);

    hboxLayout7 = new QHBoxLayout();
    hboxLayout7->setSpacing(6);
    hboxLayout7->setMargin(0);
    hboxLayout7->setObjectName(QString::fromUtf8("hboxLayout7"));
    label_9 = new QLabel(groupBox_5);
    label_9->setObjectName(QString::fromUtf8("label_9"));
    label_9->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

    hboxLayout7->addWidget(label_9);

    _prfSourceText = new QLabel(groupBox_5);
    _prfSourceText->setObjectName(QString::fromUtf8("_prfSourceText"));

    hboxLayout7->addWidget(_prfSourceText);


    hboxLayout2->addLayout(hboxLayout7);


    vboxLayout->addWidget(groupBox_5);

    groupBox = new QGroupBox(CP2Exec);
    groupBox->setObjectName(QString::fromUtf8("groupBox"));
    gridLayout = new QGridLayout(groupBox);
    gridLayout->setSpacing(6);
    gridLayout->setMargin(9);
    gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
    textLabel1_5_3 = new QLabel(groupBox);
    textLabel1_5_3->setObjectName(QString::fromUtf8("textLabel1_5_3"));
    QSizePolicy sizePolicy3(static_cast<QSizePolicy::Policy>(1), static_cast<QSizePolicy::Policy>(5));
    sizePolicy3.setHorizontalStretch(0);
    sizePolicy3.setVerticalStretch(0);
    sizePolicy3.setHeightForWidth(textLabel1_5_3->sizePolicy().hasHeightForWidth());
    textLabel1_5_3->setSizePolicy(sizePolicy3);
    textLabel1_5_3->setAlignment(Qt::AlignCenter);
    textLabel1_5_3->setWordWrap(true);

    gridLayout->addWidget(textLabel1_5_3, 3, 0, 1, 1);

    textLabel1_5_2 = new QLabel(groupBox);
    textLabel1_5_2->setObjectName(QString::fromUtf8("textLabel1_5_2"));
    QSizePolicy sizePolicy4(static_cast<QSizePolicy::Policy>(1), static_cast<QSizePolicy::Policy>(5));
    sizePolicy4.setHorizontalStretch(0);
    sizePolicy4.setVerticalStretch(0);
    sizePolicy4.setHeightForWidth(textLabel1_5_2->sizePolicy().hasHeightForWidth());
    textLabel1_5_2->setSizePolicy(sizePolicy4);
    textLabel1_5_2->setAlignment(Qt::AlignCenter);
    textLabel1_5_2->setWordWrap(true);

    gridLayout->addWidget(textLabel1_5_2, 2, 0, 1, 1);

    _chan2pulseCount = new QLabel(groupBox);
    _chan2pulseCount->setObjectName(QString::fromUtf8("_chan2pulseCount"));
    QSizePolicy sizePolicy5(static_cast<QSizePolicy::Policy>(1), static_cast<QSizePolicy::Policy>(5));
    sizePolicy5.setHorizontalStretch(0);
    sizePolicy5.setVerticalStretch(0);
    sizePolicy5.setHeightForWidth(_chan2pulseCount->sizePolicy().hasHeightForWidth());
    _chan2pulseCount->setSizePolicy(sizePolicy5);
    _chan2pulseCount->setAlignment(Qt::AlignCenter);
    _chan2pulseCount->setWordWrap(false);

    gridLayout->addWidget(_chan2pulseCount, 3, 1, 1, 1);

    _chan1pulseRate = new QLabel(groupBox);
    _chan1pulseRate->setObjectName(QString::fromUtf8("_chan1pulseRate"));
    QSizePolicy sizePolicy6(static_cast<QSizePolicy::Policy>(1), static_cast<QSizePolicy::Policy>(5));
    sizePolicy6.setHorizontalStretch(0);
    sizePolicy6.setVerticalStretch(0);
    sizePolicy6.setHeightForWidth(_chan1pulseRate->sizePolicy().hasHeightForWidth());
    _chan1pulseRate->setSizePolicy(sizePolicy6);
    _chan1pulseRate->setAlignment(Qt::AlignCenter);
    _chan1pulseRate->setWordWrap(false);

    gridLayout->addWidget(_chan1pulseRate, 2, 2, 1, 1);

    _chan0led = new QLabel(groupBox);
    _chan0led->setObjectName(QString::fromUtf8("_chan0led"));
    QSizePolicy sizePolicy7(static_cast<QSizePolicy::Policy>(0), static_cast<QSizePolicy::Policy>(0));
    sizePolicy7.setHorizontalStretch(0);
    sizePolicy7.setVerticalStretch(0);
    sizePolicy7.setHeightForWidth(_chan0led->sizePolicy().hasHeightForWidth());
    _chan0led->setSizePolicy(sizePolicy7);
    _chan0led->setMinimumSize(QSize(16, 16));
    _chan0led->setMaximumSize(QSize(16, 16));
    _chan0led->setFocusPolicy(Qt::NoFocus);
    _chan0led->setFrameShape(QFrame::Panel);
    _chan0led->setFrameShadow(QFrame::Raised);
    _chan0led->setWordWrap(false);

    gridLayout->addWidget(_chan0led, 1, 4, 1, 1);

    textLabel3 = new QLabel(groupBox);
    textLabel3->setObjectName(QString::fromUtf8("textLabel3"));
    QSizePolicy sizePolicy8(static_cast<QSizePolicy::Policy>(1), static_cast<QSizePolicy::Policy>(5));
    sizePolicy8.setHorizontalStretch(0);
    sizePolicy8.setVerticalStretch(0);
    sizePolicy8.setHeightForWidth(textLabel3->sizePolicy().hasHeightForWidth());
    textLabel3->setSizePolicy(sizePolicy8);
    textLabel3->setAlignment(Qt::AlignCenter);
    textLabel3->setWordWrap(true);

    gridLayout->addWidget(textLabel3, 0, 1, 1, 1);

    _chan2pulseRate = new QLabel(groupBox);
    _chan2pulseRate->setObjectName(QString::fromUtf8("_chan2pulseRate"));
    QSizePolicy sizePolicy9(static_cast<QSizePolicy::Policy>(1), static_cast<QSizePolicy::Policy>(5));
    sizePolicy9.setHorizontalStretch(0);
    sizePolicy9.setVerticalStretch(0);
    sizePolicy9.setHeightForWidth(_chan2pulseRate->sizePolicy().hasHeightForWidth());
    _chan2pulseRate->setSizePolicy(sizePolicy9);
    _chan2pulseRate->setAlignment(Qt::AlignCenter);
    _chan2pulseRate->setWordWrap(false);

    gridLayout->addWidget(_chan2pulseRate, 3, 2, 1, 1);

    textLabel1_5 = new QLabel(groupBox);
    textLabel1_5->setObjectName(QString::fromUtf8("textLabel1_5"));
    QSizePolicy sizePolicy10(static_cast<QSizePolicy::Policy>(1), static_cast<QSizePolicy::Policy>(5));
    sizePolicy10.setHorizontalStretch(0);
    sizePolicy10.setVerticalStretch(0);
    sizePolicy10.setHeightForWidth(textLabel1_5->sizePolicy().hasHeightForWidth());
    textLabel1_5->setSizePolicy(sizePolicy10);
    textLabel1_5->setAlignment(Qt::AlignCenter);
    textLabel1_5->setWordWrap(true);

    gridLayout->addWidget(textLabel1_5, 1, 0, 1, 1);

    textLabel3_3 = new QLabel(groupBox);
    textLabel3_3->setObjectName(QString::fromUtf8("textLabel3_3"));
    QSizePolicy sizePolicy11(static_cast<QSizePolicy::Policy>(1), static_cast<QSizePolicy::Policy>(5));
    sizePolicy11.setHorizontalStretch(0);
    sizePolicy11.setVerticalStretch(0);
    sizePolicy11.setHeightForWidth(textLabel3_3->sizePolicy().hasHeightForWidth());
    textLabel3_3->setSizePolicy(sizePolicy11);
    textLabel3_3->setAlignment(Qt::AlignCenter);
    textLabel3_3->setWordWrap(true);

    gridLayout->addWidget(textLabel3_3, 0, 3, 1, 1);

    textLabel3_4 = new QLabel(groupBox);
    textLabel3_4->setObjectName(QString::fromUtf8("textLabel3_4"));
    QSizePolicy sizePolicy12(static_cast<QSizePolicy::Policy>(1), static_cast<QSizePolicy::Policy>(5));
    sizePolicy12.setHorizontalStretch(0);
    sizePolicy12.setVerticalStretch(0);
    sizePolicy12.setHeightForWidth(textLabel3_4->sizePolicy().hasHeightForWidth());
    textLabel3_4->setSizePolicy(sizePolicy12);
    textLabel3_4->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignVCenter);
    textLabel3_4->setWordWrap(true);

    gridLayout->addWidget(textLabel3_4, 0, 4, 1, 1);

    _chan0pulseCount = new QLabel(groupBox);
    _chan0pulseCount->setObjectName(QString::fromUtf8("_chan0pulseCount"));
    QSizePolicy sizePolicy13(static_cast<QSizePolicy::Policy>(1), static_cast<QSizePolicy::Policy>(5));
    sizePolicy13.setHorizontalStretch(0);
    sizePolicy13.setVerticalStretch(0);
    sizePolicy13.setHeightForWidth(_chan0pulseCount->sizePolicy().hasHeightForWidth());
    _chan0pulseCount->setSizePolicy(sizePolicy13);
    _chan0pulseCount->setAlignment(Qt::AlignCenter);
    _chan0pulseCount->setWordWrap(false);

    gridLayout->addWidget(_chan0pulseCount, 1, 1, 1, 1);

    textLabel4 = new QLabel(groupBox);
    textLabel4->setObjectName(QString::fromUtf8("textLabel4"));
    QSizePolicy sizePolicy14(static_cast<QSizePolicy::Policy>(1), static_cast<QSizePolicy::Policy>(5));
    sizePolicy14.setHorizontalStretch(0);
    sizePolicy14.setVerticalStretch(0);
    sizePolicy14.setHeightForWidth(textLabel4->sizePolicy().hasHeightForWidth());
    textLabel4->setSizePolicy(sizePolicy14);
    textLabel4->setTextFormat(Qt::AutoText);
    textLabel4->setAlignment(Qt::AlignCenter);
    textLabel4->setWordWrap(true);

    gridLayout->addWidget(textLabel4, 0, 0, 1, 1);

    textLabel3_2 = new QLabel(groupBox);
    textLabel3_2->setObjectName(QString::fromUtf8("textLabel3_2"));
    QSizePolicy sizePolicy15(static_cast<QSizePolicy::Policy>(1), static_cast<QSizePolicy::Policy>(5));
    sizePolicy15.setHorizontalStretch(0);
    sizePolicy15.setVerticalStretch(0);
    sizePolicy15.setHeightForWidth(textLabel3_2->sizePolicy().hasHeightForWidth());
    textLabel3_2->setSizePolicy(sizePolicy15);
    textLabel3_2->setAlignment(Qt::AlignCenter);
    textLabel3_2->setWordWrap(true);

    gridLayout->addWidget(textLabel3_2, 0, 2, 1, 1);

    _chan2led = new QLabel(groupBox);
    _chan2led->setObjectName(QString::fromUtf8("_chan2led"));
    QSizePolicy sizePolicy16(static_cast<QSizePolicy::Policy>(0), static_cast<QSizePolicy::Policy>(0));
    sizePolicy16.setHorizontalStretch(0);
    sizePolicy16.setVerticalStretch(0);
    sizePolicy16.setHeightForWidth(_chan2led->sizePolicy().hasHeightForWidth());
    _chan2led->setSizePolicy(sizePolicy16);
    _chan2led->setMinimumSize(QSize(16, 16));
    _chan2led->setMaximumSize(QSize(16, 16));
    _chan2led->setFocusPolicy(Qt::NoFocus);
    _chan2led->setFrameShape(QFrame::Panel);
    _chan2led->setFrameShadow(QFrame::Raised);
    _chan2led->setWordWrap(false);

    gridLayout->addWidget(_chan2led, 3, 4, 1, 1);

    _chan1led = new QLabel(groupBox);
    _chan1led->setObjectName(QString::fromUtf8("_chan1led"));
    QSizePolicy sizePolicy17(static_cast<QSizePolicy::Policy>(0), static_cast<QSizePolicy::Policy>(0));
    sizePolicy17.setHorizontalStretch(0);
    sizePolicy17.setVerticalStretch(0);
    sizePolicy17.setHeightForWidth(_chan1led->sizePolicy().hasHeightForWidth());
    _chan1led->setSizePolicy(sizePolicy17);
    _chan1led->setMinimumSize(QSize(16, 16));
    _chan1led->setMaximumSize(QSize(16, 16));
    _chan1led->setFocusPolicy(Qt::NoFocus);
    _chan1led->setFrameShape(QFrame::Panel);
    _chan1led->setFrameShadow(QFrame::Raised);
    _chan1led->setWordWrap(false);

    gridLayout->addWidget(_chan1led, 2, 4, 1, 1);

    _chan2errors = new QLabel(groupBox);
    _chan2errors->setObjectName(QString::fromUtf8("_chan2errors"));
    QSizePolicy sizePolicy18(static_cast<QSizePolicy::Policy>(1), static_cast<QSizePolicy::Policy>(5));
    sizePolicy18.setHorizontalStretch(0);
    sizePolicy18.setVerticalStretch(0);
    sizePolicy18.setHeightForWidth(_chan2errors->sizePolicy().hasHeightForWidth());
    _chan2errors->setSizePolicy(sizePolicy18);
    _chan2errors->setAlignment(Qt::AlignCenter);
    _chan2errors->setWordWrap(false);

    gridLayout->addWidget(_chan2errors, 3, 3, 1, 1);

    _chan0errors = new QLabel(groupBox);
    _chan0errors->setObjectName(QString::fromUtf8("_chan0errors"));
    QSizePolicy sizePolicy19(static_cast<QSizePolicy::Policy>(1), static_cast<QSizePolicy::Policy>(5));
    sizePolicy19.setHorizontalStretch(0);
    sizePolicy19.setVerticalStretch(0);
    sizePolicy19.setHeightForWidth(_chan0errors->sizePolicy().hasHeightForWidth());
    _chan0errors->setSizePolicy(sizePolicy19);
    _chan0errors->setAlignment(Qt::AlignCenter);
    _chan0errors->setWordWrap(false);

    gridLayout->addWidget(_chan0errors, 1, 3, 1, 1);

    _chan1pulseCount = new QLabel(groupBox);
    _chan1pulseCount->setObjectName(QString::fromUtf8("_chan1pulseCount"));
    QSizePolicy sizePolicy20(static_cast<QSizePolicy::Policy>(1), static_cast<QSizePolicy::Policy>(5));
    sizePolicy20.setHorizontalStretch(0);
    sizePolicy20.setVerticalStretch(0);
    sizePolicy20.setHeightForWidth(_chan1pulseCount->sizePolicy().hasHeightForWidth());
    _chan1pulseCount->setSizePolicy(sizePolicy20);
    _chan1pulseCount->setAlignment(Qt::AlignCenter);
    _chan1pulseCount->setWordWrap(false);

    gridLayout->addWidget(_chan1pulseCount, 2, 1, 1, 1);

    _chan1errors = new QLabel(groupBox);
    _chan1errors->setObjectName(QString::fromUtf8("_chan1errors"));
    QSizePolicy sizePolicy21(static_cast<QSizePolicy::Policy>(1), static_cast<QSizePolicy::Policy>(5));
    sizePolicy21.setHorizontalStretch(0);
    sizePolicy21.setVerticalStretch(0);
    sizePolicy21.setHeightForWidth(_chan1errors->sizePolicy().hasHeightForWidth());
    _chan1errors->setSizePolicy(sizePolicy21);
    _chan1errors->setAlignment(Qt::AlignCenter);
    _chan1errors->setWordWrap(false);

    gridLayout->addWidget(_chan1errors, 2, 3, 1, 1);

    _chan0pulseRate = new QLabel(groupBox);
    _chan0pulseRate->setObjectName(QString::fromUtf8("_chan0pulseRate"));
    QSizePolicy sizePolicy22(static_cast<QSizePolicy::Policy>(1), static_cast<QSizePolicy::Policy>(5));
    sizePolicy22.setHorizontalStretch(0);
    sizePolicy22.setVerticalStretch(0);
    sizePolicy22.setHeightForWidth(_chan0pulseRate->sizePolicy().hasHeightForWidth());
    _chan0pulseRate->setSizePolicy(sizePolicy22);
    _chan0pulseRate->setAlignment(Qt::AlignCenter);
    _chan0pulseRate->setWordWrap(false);

    gridLayout->addWidget(_chan0pulseRate, 1, 2, 1, 1);


    vboxLayout->addWidget(groupBox);

    groupBox_3 = new QGroupBox(CP2Exec);
    groupBox_3->setObjectName(QString::fromUtf8("groupBox_3"));
    gridLayout1 = new QGridLayout(groupBox_3);
    gridLayout1->setSpacing(6);
    gridLayout1->setMargin(9);
    gridLayout1->setObjectName(QString::fromUtf8("gridLayout1"));
    textLabel3_6 = new QLabel(groupBox_3);
    textLabel3_6->setObjectName(QString::fromUtf8("textLabel3_6"));
    QSizePolicy sizePolicy23(static_cast<QSizePolicy::Policy>(1), static_cast<QSizePolicy::Policy>(5));
    sizePolicy23.setHorizontalStretch(0);
    sizePolicy23.setVerticalStretch(0);
    sizePolicy23.setHeightForWidth(textLabel3_6->sizePolicy().hasHeightForWidth());
    textLabel3_6->setSizePolicy(sizePolicy23);
    textLabel3_6->setAlignment(Qt::AlignCenter);
    textLabel3_6->setWordWrap(true);

    gridLayout1->addWidget(textLabel3_6, 0, 4, 1, 1);

    _chan2Volume = new QLabel(groupBox_3);
    _chan2Volume->setObjectName(QString::fromUtf8("_chan2Volume"));
    _chan2Volume->setAlignment(Qt::AlignCenter);

    gridLayout1->addWidget(_chan2Volume, 3, 4, 1, 1);

    _chan0Volume = new QLabel(groupBox_3);
    _chan0Volume->setObjectName(QString::fromUtf8("_chan0Volume"));
    _chan0Volume->setAlignment(Qt::AlignCenter);

    gridLayout1->addWidget(_chan0Volume, 1, 4, 1, 1);

    _chan1Volume = new QLabel(groupBox_3);
    _chan1Volume->setObjectName(QString::fromUtf8("_chan1Volume"));
    _chan1Volume->setAlignment(Qt::AlignCenter);

    gridLayout1->addWidget(_chan1Volume, 2, 4, 1, 1);

    _chan2Sweep = new QLabel(groupBox_3);
    _chan2Sweep->setObjectName(QString::fromUtf8("_chan2Sweep"));
    QSizePolicy sizePolicy24(static_cast<QSizePolicy::Policy>(1), static_cast<QSizePolicy::Policy>(5));
    sizePolicy24.setHorizontalStretch(0);
    sizePolicy24.setVerticalStretch(0);
    sizePolicy24.setHeightForWidth(_chan2Sweep->sizePolicy().hasHeightForWidth());
    _chan2Sweep->setSizePolicy(sizePolicy24);
    _chan2Sweep->setAlignment(Qt::AlignCenter);
    _chan2Sweep->setWordWrap(false);

    gridLayout1->addWidget(_chan2Sweep, 3, 3, 1, 1);

    _chan1Sweep = new QLabel(groupBox_3);
    _chan1Sweep->setObjectName(QString::fromUtf8("_chan1Sweep"));
    QSizePolicy sizePolicy25(static_cast<QSizePolicy::Policy>(1), static_cast<QSizePolicy::Policy>(5));
    sizePolicy25.setHorizontalStretch(0);
    sizePolicy25.setVerticalStretch(0);
    sizePolicy25.setHeightForWidth(_chan1Sweep->sizePolicy().hasHeightForWidth());
    _chan1Sweep->setSizePolicy(sizePolicy25);
    _chan1Sweep->setAlignment(Qt::AlignCenter);
    _chan1Sweep->setWordWrap(false);

    gridLayout1->addWidget(_chan1Sweep, 2, 3, 1, 1);

    _chan0Sweep = new QLabel(groupBox_3);
    _chan0Sweep->setObjectName(QString::fromUtf8("_chan0Sweep"));
    QSizePolicy sizePolicy26(static_cast<QSizePolicy::Policy>(1), static_cast<QSizePolicy::Policy>(5));
    sizePolicy26.setHorizontalStretch(0);
    sizePolicy26.setVerticalStretch(0);
    sizePolicy26.setHeightForWidth(_chan0Sweep->sizePolicy().hasHeightForWidth());
    _chan0Sweep->setSizePolicy(sizePolicy26);
    _chan0Sweep->setAlignment(Qt::AlignCenter);
    _chan0Sweep->setWordWrap(false);

    gridLayout1->addWidget(_chan0Sweep, 1, 3, 1, 1);

    _chan2El = new QLabel(groupBox_3);
    _chan2El->setObjectName(QString::fromUtf8("_chan2El"));
    QSizePolicy sizePolicy27(static_cast<QSizePolicy::Policy>(1), static_cast<QSizePolicy::Policy>(5));
    sizePolicy27.setHorizontalStretch(0);
    sizePolicy27.setVerticalStretch(0);
    sizePolicy27.setHeightForWidth(_chan2El->sizePolicy().hasHeightForWidth());
    _chan2El->setSizePolicy(sizePolicy27);
    _chan2El->setAlignment(Qt::AlignCenter);
    _chan2El->setWordWrap(false);

    gridLayout1->addWidget(_chan2El, 3, 2, 1, 1);

    _chan1El = new QLabel(groupBox_3);
    _chan1El->setObjectName(QString::fromUtf8("_chan1El"));
    QSizePolicy sizePolicy28(static_cast<QSizePolicy::Policy>(1), static_cast<QSizePolicy::Policy>(5));
    sizePolicy28.setHorizontalStretch(0);
    sizePolicy28.setVerticalStretch(0);
    sizePolicy28.setHeightForWidth(_chan1El->sizePolicy().hasHeightForWidth());
    _chan1El->setSizePolicy(sizePolicy28);
    _chan1El->setAlignment(Qt::AlignCenter);
    _chan1El->setWordWrap(false);

    gridLayout1->addWidget(_chan1El, 2, 2, 1, 1);

    _chan0El = new QLabel(groupBox_3);
    _chan0El->setObjectName(QString::fromUtf8("_chan0El"));
    QSizePolicy sizePolicy29(static_cast<QSizePolicy::Policy>(1), static_cast<QSizePolicy::Policy>(5));
    sizePolicy29.setHorizontalStretch(0);
    sizePolicy29.setVerticalStretch(0);
    sizePolicy29.setHeightForWidth(_chan0El->sizePolicy().hasHeightForWidth());
    _chan0El->setSizePolicy(sizePolicy29);
    _chan0El->setAlignment(Qt::AlignCenter);
    _chan0El->setWordWrap(false);

    gridLayout1->addWidget(_chan0El, 1, 2, 1, 1);

    _chan0Az = new QLabel(groupBox_3);
    _chan0Az->setObjectName(QString::fromUtf8("_chan0Az"));
    QSizePolicy sizePolicy30(static_cast<QSizePolicy::Policy>(1), static_cast<QSizePolicy::Policy>(5));
    sizePolicy30.setHorizontalStretch(0);
    sizePolicy30.setVerticalStretch(0);
    sizePolicy30.setHeightForWidth(_chan0Az->sizePolicy().hasHeightForWidth());
    _chan0Az->setSizePolicy(sizePolicy30);
    _chan0Az->setAlignment(Qt::AlignCenter);
    _chan0Az->setWordWrap(false);

    gridLayout1->addWidget(_chan0Az, 1, 1, 1, 1);

    _chan1Az = new QLabel(groupBox_3);
    _chan1Az->setObjectName(QString::fromUtf8("_chan1Az"));
    QSizePolicy sizePolicy31(static_cast<QSizePolicy::Policy>(1), static_cast<QSizePolicy::Policy>(5));
    sizePolicy31.setHorizontalStretch(0);
    sizePolicy31.setVerticalStretch(0);
    sizePolicy31.setHeightForWidth(_chan1Az->sizePolicy().hasHeightForWidth());
    _chan1Az->setSizePolicy(sizePolicy31);
    _chan1Az->setAlignment(Qt::AlignCenter);
    _chan1Az->setWordWrap(false);

    gridLayout1->addWidget(_chan1Az, 2, 1, 1, 1);

    _chan2Az = new QLabel(groupBox_3);
    _chan2Az->setObjectName(QString::fromUtf8("_chan2Az"));
    QSizePolicy sizePolicy32(static_cast<QSizePolicy::Policy>(1), static_cast<QSizePolicy::Policy>(5));
    sizePolicy32.setHorizontalStretch(0);
    sizePolicy32.setVerticalStretch(0);
    sizePolicy32.setHeightForWidth(_chan2Az->sizePolicy().hasHeightForWidth());
    _chan2Az->setSizePolicy(sizePolicy32);
    _chan2Az->setAlignment(Qt::AlignCenter);
    _chan2Az->setWordWrap(false);

    gridLayout1->addWidget(_chan2Az, 3, 1, 1, 1);

    textLabel3_7 = new QLabel(groupBox_3);
    textLabel3_7->setObjectName(QString::fromUtf8("textLabel3_7"));
    QSizePolicy sizePolicy33(static_cast<QSizePolicy::Policy>(1), static_cast<QSizePolicy::Policy>(5));
    sizePolicy33.setHorizontalStretch(0);
    sizePolicy33.setVerticalStretch(0);
    sizePolicy33.setHeightForWidth(textLabel3_7->sizePolicy().hasHeightForWidth());
    textLabel3_7->setSizePolicy(sizePolicy33);
    textLabel3_7->setAlignment(Qt::AlignCenter);
    textLabel3_7->setWordWrap(true);

    gridLayout1->addWidget(textLabel3_7, 0, 3, 1, 1);

    textLabel3_5 = new QLabel(groupBox_3);
    textLabel3_5->setObjectName(QString::fromUtf8("textLabel3_5"));
    QSizePolicy sizePolicy34(static_cast<QSizePolicy::Policy>(1), static_cast<QSizePolicy::Policy>(5));
    sizePolicy34.setHorizontalStretch(0);
    sizePolicy34.setVerticalStretch(0);
    sizePolicy34.setHeightForWidth(textLabel3_5->sizePolicy().hasHeightForWidth());
    textLabel3_5->setSizePolicy(sizePolicy34);
    textLabel3_5->setAlignment(Qt::AlignCenter);
    textLabel3_5->setWordWrap(true);

    gridLayout1->addWidget(textLabel3_5, 0, 2, 1, 1);

    textLabel3_8 = new QLabel(groupBox_3);
    textLabel3_8->setObjectName(QString::fromUtf8("textLabel3_8"));
    QSizePolicy sizePolicy35(static_cast<QSizePolicy::Policy>(1), static_cast<QSizePolicy::Policy>(5));
    sizePolicy35.setHorizontalStretch(0);
    sizePolicy35.setVerticalStretch(0);
    sizePolicy35.setHeightForWidth(textLabel3_8->sizePolicy().hasHeightForWidth());
    textLabel3_8->setSizePolicy(sizePolicy35);
    textLabel3_8->setAlignment(Qt::AlignCenter);
    textLabel3_8->setWordWrap(true);

    gridLayout1->addWidget(textLabel3_8, 0, 1, 1, 1);

    textLabel1_5_5 = new QLabel(groupBox_3);
    textLabel1_5_5->setObjectName(QString::fromUtf8("textLabel1_5_5"));
    QSizePolicy sizePolicy36(static_cast<QSizePolicy::Policy>(1), static_cast<QSizePolicy::Policy>(5));
    sizePolicy36.setHorizontalStretch(0);
    sizePolicy36.setVerticalStretch(0);
    sizePolicy36.setHeightForWidth(textLabel1_5_5->sizePolicy().hasHeightForWidth());
    textLabel1_5_5->setSizePolicy(sizePolicy36);
    textLabel1_5_5->setAlignment(Qt::AlignCenter);
    textLabel1_5_5->setWordWrap(true);

    gridLayout1->addWidget(textLabel1_5_5, 3, 0, 1, 1);

    textLabel1_5_4 = new QLabel(groupBox_3);
    textLabel1_5_4->setObjectName(QString::fromUtf8("textLabel1_5_4"));
    QSizePolicy sizePolicy37(static_cast<QSizePolicy::Policy>(1), static_cast<QSizePolicy::Policy>(5));
    sizePolicy37.setHorizontalStretch(0);
    sizePolicy37.setVerticalStretch(0);
    sizePolicy37.setHeightForWidth(textLabel1_5_4->sizePolicy().hasHeightForWidth());
    textLabel1_5_4->setSizePolicy(sizePolicy37);
    textLabel1_5_4->setAlignment(Qt::AlignCenter);
    textLabel1_5_4->setWordWrap(true);

    gridLayout1->addWidget(textLabel1_5_4, 2, 0, 1, 1);

    textLabel1_6 = new QLabel(groupBox_3);
    textLabel1_6->setObjectName(QString::fromUtf8("textLabel1_6"));
    QSizePolicy sizePolicy38(static_cast<QSizePolicy::Policy>(1), static_cast<QSizePolicy::Policy>(5));
    sizePolicy38.setHorizontalStretch(0);
    sizePolicy38.setVerticalStretch(0);
    sizePolicy38.setHeightForWidth(textLabel1_6->sizePolicy().hasHeightForWidth());
    textLabel1_6->setSizePolicy(sizePolicy38);
    textLabel1_6->setAlignment(Qt::AlignCenter);
    textLabel1_6->setWordWrap(true);

    gridLayout1->addWidget(textLabel1_6, 1, 0, 1, 1);

    textLabel4_2 = new QLabel(groupBox_3);
    textLabel4_2->setObjectName(QString::fromUtf8("textLabel4_2"));
    QSizePolicy sizePolicy39(static_cast<QSizePolicy::Policy>(1), static_cast<QSizePolicy::Policy>(5));
    sizePolicy39.setHorizontalStretch(0);
    sizePolicy39.setVerticalStretch(0);
    sizePolicy39.setHeightForWidth(textLabel4_2->sizePolicy().hasHeightForWidth());
    textLabel4_2->setSizePolicy(sizePolicy39);
    textLabel4_2->setTextFormat(Qt::AutoText);
    textLabel4_2->setAlignment(Qt::AlignCenter);
    textLabel4_2->setWordWrap(true);

    gridLayout1->addWidget(textLabel4_2, 0, 0, 1, 1);


    vboxLayout->addWidget(groupBox_3);

    groupBox_4 = new QGroupBox(CP2Exec);
    groupBox_4->setObjectName(QString::fromUtf8("groupBox_4"));
    hboxLayout8 = new QHBoxLayout(groupBox_4);
    hboxLayout8->setSpacing(6);
    hboxLayout8->setMargin(9);
    hboxLayout8->setObjectName(QString::fromUtf8("hboxLayout8"));
    _networkIP = new QLabel(groupBox_4);
    _networkIP->setObjectName(QString::fromUtf8("_networkIP"));

    hboxLayout8->addWidget(_networkIP);

    _networkPort = new QLabel(groupBox_4);
    _networkPort->setObjectName(QString::fromUtf8("_networkPort"));

    hboxLayout8->addWidget(_networkPort);

    spacerItem = new QSpacerItem(171, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

    hboxLayout8->addItem(spacerItem);


    vboxLayout->addWidget(groupBox_4);

    buttonBox = new QDialogButtonBox(CP2Exec);
    buttonBox->setObjectName(QString::fromUtf8("buttonBox"));
    buttonBox->setOrientation(Qt::Horizontal);
    buttonBox->setStandardButtons(QDialogButtonBox::Close);
    buttonBox->setCenterButtons(true);

    vboxLayout->addWidget(buttonBox);


    retranslateUi(CP2Exec);

    QSize size(754, 484);
    size = size.expandedTo(CP2Exec->minimumSizeHint());
    CP2Exec->resize(size);

    QObject::connect(buttonBox, SIGNAL(accepted()), CP2Exec, SLOT(accept()));
    QObject::connect(buttonBox, SIGNAL(rejected()), CP2Exec, SLOT(reject()));

    QMetaObject::connectSlotsByName(CP2Exec);
    } // setupUi

    void retranslateUi(QDialog *CP2Exec)
    {
    CP2Exec->setWindowTitle(QApplication::translate("CP2Exec", "CP2Exec", 0, QApplication::UnicodeUTF8));
    groupBox_2->setTitle(QApplication::translate("CP2Exec", "Status", 0, QApplication::UnicodeUTF8));
    _statusText->setText(QApplication::translate("CP2Exec", "status", 0, QApplication::UnicodeUTF8));
    label->setText(QApplication::translate("CP2Exec", "Elapsed Time (hours):", 0, QApplication::UnicodeUTF8));
    _elapsedText->setText(QApplication::translate("CP2Exec", "0.0", 0, QApplication::UnicodeUTF8));
    groupBox_5->setTitle(QApplication::translate("CP2Exec", "Parameters", 0, QApplication::UnicodeUTF8));
    label_2->setText(QApplication::translate("CP2Exec", "Gates:", 0, QApplication::UnicodeUTF8));
    _gatesText->setText(QApplication::translate("CP2Exec", "0", 0, QApplication::UnicodeUTF8));
    label_4->setText(QApplication::translate("CP2Exec", "      PRF (Hz):", 0, QApplication::UnicodeUTF8));
    _prfHzText->setText(QApplication::translate("CP2Exec", "0", 0, QApplication::UnicodeUTF8));
    label_6->setText(QApplication::translate("CP2Exec", "         Pulse Width (uS):", 0, QApplication::UnicodeUTF8));
    _pwText->setText(QApplication::translate("CP2Exec", "0", 0, QApplication::UnicodeUTF8));
    label_8->setText(QApplication::translate("CP2Exec", "         Simulated Antenna Angles:", 0, QApplication::UnicodeUTF8));
    _simAnglesText->setText(QApplication::translate("CP2Exec", "Off", 0, QApplication::UnicodeUTF8));
    label_9->setText(QApplication::translate("CP2Exec", "           PRF Source:", 0, QApplication::UnicodeUTF8));
    _prfSourceText->setText(QApplication::translate("CP2Exec", "Unknown", 0, QApplication::UnicodeUTF8));
    groupBox->setTitle(QApplication::translate("CP2Exec", "Pulse Statistics", 0, QApplication::UnicodeUTF8));
    textLabel1_5_3->setText(QApplication::translate("CP2Exec", "<font size=\"+1\"><b>Xv</b></font>", 0, QApplication::UnicodeUTF8));
    textLabel1_5_2->setText(QApplication::translate("CP2Exec", "<font size=\"+1\"><b>Xh</b></font>", 0, QApplication::UnicodeUTF8));
    _chan2pulseCount->setText(QApplication::translate("CP2Exec", "0", 0, QApplication::UnicodeUTF8));
    _chan1pulseRate->setText(QApplication::translate("CP2Exec", "0", 0, QApplication::UnicodeUTF8));
    _chan0led->setText(QString());
    textLabel3->setText(QApplication::translate("CP2Exec", "<font size=\"+1\"><p align=\"center\"><b>Number<br>\n"
"\n"
"(K)</b></p></font>", 0, QApplication::UnicodeUTF8));
    _chan2pulseRate->setText(QApplication::translate("CP2Exec", "0", 0, QApplication::UnicodeUTF8));
    textLabel1_5->setText(QApplication::translate("CP2Exec", "<font size=\"+1\"><b>S</b></font>", 0, QApplication::UnicodeUTF8));
    textLabel3_3->setText(QApplication::translate("CP2Exec", "<font size=\"+1\"><b>Errors</b></font>", 0, QApplication::UnicodeUTF8));
    textLabel3_4->setText(QApplication::translate("CP2Exec", "<font size=\"+1\"><b>EOF</b></font>", 0, QApplication::UnicodeUTF8));
    _chan0pulseCount->setText(QApplication::translate("CP2Exec", "0", 0, QApplication::UnicodeUTF8));
    textLabel4->setText(QApplication::translate("CP2Exec", "<font size=\"+1\"><b>Channel</b>    </font>", 0, QApplication::UnicodeUTF8));
    textLabel3_2->setText(QApplication::translate("CP2Exec", "<font size=\"+1\"><b>Rate<br>\n"
"\n"
"(/s)</b></font>", 0, QApplication::UnicodeUTF8));
    _chan2led->setText(QString());
    _chan1led->setText(QString());
    _chan2errors->setText(QApplication::translate("CP2Exec", "0", 0, QApplication::UnicodeUTF8));
    _chan0errors->setText(QApplication::translate("CP2Exec", "0", 0, QApplication::UnicodeUTF8));
    _chan1pulseCount->setText(QApplication::translate("CP2Exec", "0", 0, QApplication::UnicodeUTF8));
    _chan1errors->setText(QApplication::translate("CP2Exec", "0", 0, QApplication::UnicodeUTF8));
    _chan0pulseRate->setText(QApplication::translate("CP2Exec", "0", 0, QApplication::UnicodeUTF8));
    groupBox_3->setTitle(QApplication::translate("CP2Exec", "Antenna Readings", 0, QApplication::UnicodeUTF8));
    textLabel3_6->setText(QApplication::translate("CP2Exec", "<font size=\"+1\"><b>Volume</b></font>", 0, QApplication::UnicodeUTF8));
    _chan2Volume->setText(QApplication::translate("CP2Exec", "0", 0, QApplication::UnicodeUTF8));
    _chan0Volume->setText(QApplication::translate("CP2Exec", "0", 0, QApplication::UnicodeUTF8));
    _chan1Volume->setText(QApplication::translate("CP2Exec", "0", 0, QApplication::UnicodeUTF8));
    _chan2Sweep->setText(QApplication::translate("CP2Exec", "0", 0, QApplication::UnicodeUTF8));
    _chan1Sweep->setText(QApplication::translate("CP2Exec", "0", 0, QApplication::UnicodeUTF8));
    _chan0Sweep->setText(QApplication::translate("CP2Exec", "0", 0, QApplication::UnicodeUTF8));
    _chan2El->setText(QApplication::translate("CP2Exec", "0", 0, QApplication::UnicodeUTF8));
    _chan1El->setText(QApplication::translate("CP2Exec", "0", 0, QApplication::UnicodeUTF8));
    _chan0El->setText(QApplication::translate("CP2Exec", "0", 0, QApplication::UnicodeUTF8));
    _chan0Az->setText(QApplication::translate("CP2Exec", "0", 0, QApplication::UnicodeUTF8));
    _chan1Az->setText(QApplication::translate("CP2Exec", "0", 0, QApplication::UnicodeUTF8));
    _chan2Az->setText(QApplication::translate("CP2Exec", "0", 0, QApplication::UnicodeUTF8));
    textLabel3_7->setText(QApplication::translate("CP2Exec", "<font size=\"+1\"><b>Sweep</b></font>", 0, QApplication::UnicodeUTF8));
    textLabel3_5->setText(QApplication::translate("CP2Exec", "<font size=\"+1\"><b>Elevation</b></font>", 0, QApplication::UnicodeUTF8));
    textLabel3_8->setText(QApplication::translate("CP2Exec", "<font size=\"+1\"><p align=\"center\"><b>Azimuth</b></p></font>", 0, QApplication::UnicodeUTF8));
    textLabel1_5_5->setText(QApplication::translate("CP2Exec", "<font size=\"+1\"><b>Xv</b></font>", 0, QApplication::UnicodeUTF8));
    textLabel1_5_4->setText(QApplication::translate("CP2Exec", "<font size=\"+1\"><b>Xh</b></font>", 0, QApplication::UnicodeUTF8));
    textLabel1_6->setText(QApplication::translate("CP2Exec", "<font size=\"+1\"><b>S</b></font>", 0, QApplication::UnicodeUTF8));
    textLabel4_2->setText(QApplication::translate("CP2Exec", "<font size=\"+1\"><b>Piraq</b>    </font>", 0, QApplication::UnicodeUTF8));
    groupBox_4->setTitle(QApplication::translate("CP2Exec", "Pulse Network Interface", 0, QApplication::UnicodeUTF8));
    _networkIP->setText(QApplication::translate("CP2Exec", "IP", 0, QApplication::UnicodeUTF8));
    _networkPort->setText(QApplication::translate("CP2Exec", "Port", 0, QApplication::UnicodeUTF8));
    Q_UNUSED(CP2Exec);
    } // retranslateUi

};

namespace Ui {
    class CP2Exec: public Ui_CP2Exec {};
} // namespace Ui

#endif // UI_CP2EXEC_H

/********************************************************************************
** Form generated from reading ui file 'CP2Scope.ui'
**
** Created: Wed Aug 1 11:46:42 2007
**      by: Qt User Interface Compiler version 4.2.2
**
** WARNING! All changes made in this file will be lost when recompiling ui file!
********************************************************************************/

#ifndef UI_CP2SCOPE_H
#define UI_CP2SCOPE_H

#include "Knob.h"
#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QCheckBox>
#include <QtGui/QGridLayout>
#include <QtGui/QGroupBox>
#include <QtGui/QHBoxLayout>
#include <QtGui/QLabel>
#include <QtGui/QPushButton>
#include <QtGui/QTabWidget>
#include <QtGui/QToolButton>
#include <QtGui/QVBoxLayout>
#include <QtGui/QWidget>
#include "ScopePlot.h"

class Ui_CP2Scope
{
public:
    QHBoxLayout *hboxLayout;
    ScopePlot *_scopePlot;
    QVBoxLayout *vboxLayout;
    QTabWidget *_typeTab;
    QWidget *Channels;
    QVBoxLayout *vboxLayout1;
    QHBoxLayout *hboxLayout1;
    QCheckBox *pauseButton;
    QCheckBox *xGrid;
    QCheckBox *yGrid;
    QHBoxLayout *hboxLayout2;
    QPushButton *_autoScale;
    QPushButton *_saveImage;
    QHBoxLayout *hboxLayout3;
    QVBoxLayout *vboxLayout2;
    QToolButton *_up;
    QToolButton *_dn;
    Knob *_gainKnob;
    QGroupBox *groupBox;
    QVBoxLayout *vboxLayout3;
    QGridLayout *gridLayout;
    QLabel *_chan2led;
    QLabel *_chan2pulseCount;
    QLabel *_chan1errors;
    QLabel *textLabel1_5;
    QLabel *_chan0errors;
    QLabel *_chan1pulseCount;
    QLabel *textLabel1_5_3;
    QLabel *textLabel3_3;
    QLabel *_chan0pulseCount;
    QLabel *_chan0led;
    QLabel *textLabel1;
    QLabel *_chan2errors;
    QLabel *textLabel4;
    QLabel *textLabel1_5_2;
    QLabel *textLabel3_2;
    QLabel *_chan2pulseRate;
    QLabel *_chan0pulseRate;
    QLabel *_chan1led;
    QLabel *textLabel3;
    QLabel *_chan1pulseRate;
    QGridLayout *gridLayout1;
    QHBoxLayout *hboxLayout4;
    QLabel *m_pTextIPname_2;
    QLabel *m_pulseIP;
    QHBoxLayout *hboxLayout5;
    QLabel *m_pTextIPname_4;
    QLabel *m_pulseDec;
    QHBoxLayout *hboxLayout6;
    QLabel *m_pTextIPname_3;
    QLabel *m_productDec;
    QHBoxLayout *hboxLayout7;
    QLabel *m_pTextIPname;
    QLabel *m_productIP;

    void setupUi(QWidget *CP2Scope)
    {
    CP2Scope->setObjectName(QString::fromUtf8("CP2Scope"));
    hboxLayout = new QHBoxLayout(CP2Scope);
    hboxLayout->setSpacing(6);
    hboxLayout->setMargin(9);
    hboxLayout->setObjectName(QString::fromUtf8("hboxLayout"));
    _scopePlot = new ScopePlot(CP2Scope);
    _scopePlot->setObjectName(QString::fromUtf8("_scopePlot"));
    QSizePolicy sizePolicy(static_cast<QSizePolicy::Policy>(3), static_cast<QSizePolicy::Policy>(3));
    sizePolicy.setHorizontalStretch(0);
    sizePolicy.setVerticalStretch(0);
    sizePolicy.setHeightForWidth(_scopePlot->sizePolicy().hasHeightForWidth());
    _scopePlot->setSizePolicy(sizePolicy);
    _scopePlot->setMinimumSize(QSize(600, 600));

    hboxLayout->addWidget(_scopePlot);

    vboxLayout = new QVBoxLayout();
    vboxLayout->setSpacing(6);
    vboxLayout->setMargin(0);
    vboxLayout->setObjectName(QString::fromUtf8("vboxLayout"));
    _typeTab = new QTabWidget(CP2Scope);
    _typeTab->setObjectName(QString::fromUtf8("_typeTab"));
    QSizePolicy sizePolicy1(static_cast<QSizePolicy::Policy>(1), static_cast<QSizePolicy::Policy>(5));
    sizePolicy1.setHorizontalStretch(0);
    sizePolicy1.setVerticalStretch(0);
    sizePolicy1.setHeightForWidth(_typeTab->sizePolicy().hasHeightForWidth());
    _typeTab->setSizePolicy(sizePolicy1);
    _typeTab->setMinimumSize(QSize(200, 300));
    _typeTab->setMaximumSize(QSize(16777215, 16777215));
    Channels = new QWidget();
    Channels->setObjectName(QString::fromUtf8("Channels"));
    _typeTab->addTab(Channels, QApplication::translate("CP2Scope", "Chan", 0, QApplication::UnicodeUTF8));

    vboxLayout->addWidget(_typeTab);

    vboxLayout1 = new QVBoxLayout();
    vboxLayout1->setSpacing(6);
    vboxLayout1->setMargin(0);
    vboxLayout1->setObjectName(QString::fromUtf8("vboxLayout1"));
    hboxLayout1 = new QHBoxLayout();
    hboxLayout1->setSpacing(6);
    hboxLayout1->setMargin(0);
    hboxLayout1->setObjectName(QString::fromUtf8("hboxLayout1"));
    pauseButton = new QCheckBox(CP2Scope);
    pauseButton->setObjectName(QString::fromUtf8("pauseButton"));
    QSizePolicy sizePolicy2(static_cast<QSizePolicy::Policy>(1), static_cast<QSizePolicy::Policy>(1));
    sizePolicy2.setHorizontalStretch(0);
    sizePolicy2.setVerticalStretch(0);
    sizePolicy2.setHeightForWidth(pauseButton->sizePolicy().hasHeightForWidth());
    pauseButton->setSizePolicy(sizePolicy2);

    hboxLayout1->addWidget(pauseButton);

    xGrid = new QCheckBox(CP2Scope);
    xGrid->setObjectName(QString::fromUtf8("xGrid"));
    QSizePolicy sizePolicy3(static_cast<QSizePolicy::Policy>(1), static_cast<QSizePolicy::Policy>(5));
    sizePolicy3.setHorizontalStretch(0);
    sizePolicy3.setVerticalStretch(0);
    sizePolicy3.setHeightForWidth(xGrid->sizePolicy().hasHeightForWidth());
    xGrid->setSizePolicy(sizePolicy3);
    xGrid->setChecked(true);

    hboxLayout1->addWidget(xGrid);

    yGrid = new QCheckBox(CP2Scope);
    yGrid->setObjectName(QString::fromUtf8("yGrid"));
    QSizePolicy sizePolicy4(static_cast<QSizePolicy::Policy>(1), static_cast<QSizePolicy::Policy>(5));
    sizePolicy4.setHorizontalStretch(0);
    sizePolicy4.setVerticalStretch(0);
    sizePolicy4.setHeightForWidth(yGrid->sizePolicy().hasHeightForWidth());
    yGrid->setSizePolicy(sizePolicy4);
    yGrid->setChecked(true);

    hboxLayout1->addWidget(yGrid);


    vboxLayout1->addLayout(hboxLayout1);

    hboxLayout2 = new QHBoxLayout();
    hboxLayout2->setSpacing(6);
    hboxLayout2->setMargin(0);
    hboxLayout2->setObjectName(QString::fromUtf8("hboxLayout2"));
    _autoScale = new QPushButton(CP2Scope);
    _autoScale->setObjectName(QString::fromUtf8("_autoScale"));
    QSizePolicy sizePolicy5(static_cast<QSizePolicy::Policy>(1), static_cast<QSizePolicy::Policy>(1));
    sizePolicy5.setHorizontalStretch(0);
    sizePolicy5.setVerticalStretch(0);
    sizePolicy5.setHeightForWidth(_autoScale->sizePolicy().hasHeightForWidth());
    _autoScale->setSizePolicy(sizePolicy5);

    hboxLayout2->addWidget(_autoScale);

    _saveImage = new QPushButton(CP2Scope);
    _saveImage->setObjectName(QString::fromUtf8("_saveImage"));

    hboxLayout2->addWidget(_saveImage);


    vboxLayout1->addLayout(hboxLayout2);


    vboxLayout->addLayout(vboxLayout1);

    hboxLayout3 = new QHBoxLayout();
    hboxLayout3->setSpacing(6);
    hboxLayout3->setMargin(0);
    hboxLayout3->setObjectName(QString::fromUtf8("hboxLayout3"));
    vboxLayout2 = new QVBoxLayout();
    vboxLayout2->setSpacing(6);
    vboxLayout2->setMargin(0);
    vboxLayout2->setObjectName(QString::fromUtf8("vboxLayout2"));
    _up = new QToolButton(CP2Scope);
    _up->setObjectName(QString::fromUtf8("_up"));
    QSizePolicy sizePolicy6(static_cast<QSizePolicy::Policy>(0), static_cast<QSizePolicy::Policy>(3));
    sizePolicy6.setHorizontalStretch(0);
    sizePolicy6.setVerticalStretch(0);
    sizePolicy6.setHeightForWidth(_up->sizePolicy().hasHeightForWidth());
    _up->setSizePolicy(sizePolicy6);
    _up->setMinimumSize(QSize(30, 0));
    _up->setMaximumSize(QSize(20, 16777215));
    _up->setAutoRepeat(true);

    vboxLayout2->addWidget(_up);

    _dn = new QToolButton(CP2Scope);
    _dn->setObjectName(QString::fromUtf8("_dn"));
    QSizePolicy sizePolicy7(static_cast<QSizePolicy::Policy>(0), static_cast<QSizePolicy::Policy>(3));
    sizePolicy7.setHorizontalStretch(0);
    sizePolicy7.setVerticalStretch(0);
    sizePolicy7.setHeightForWidth(_dn->sizePolicy().hasHeightForWidth());
    _dn->setSizePolicy(sizePolicy7);
    _dn->setMinimumSize(QSize(30, 0));
    _dn->setMaximumSize(QSize(30, 16777215));
    _dn->setAutoRepeat(true);

    vboxLayout2->addWidget(_dn);


    hboxLayout3->addLayout(vboxLayout2);

    _gainKnob = new Knob(CP2Scope);
    _gainKnob->setObjectName(QString::fromUtf8("_gainKnob"));
    QSizePolicy sizePolicy8(static_cast<QSizePolicy::Policy>(3), static_cast<QSizePolicy::Policy>(3));
    sizePolicy8.setHorizontalStretch(0);
    sizePolicy8.setVerticalStretch(0);
    sizePolicy8.setHeightForWidth(_gainKnob->sizePolicy().hasHeightForWidth());
    _gainKnob->setSizePolicy(sizePolicy8);
    _gainKnob->setMinimumSize(QSize(100, 0));
    _gainKnob->setMaximumSize(QSize(200, 16777215));

    hboxLayout3->addWidget(_gainKnob);


    vboxLayout->addLayout(hboxLayout3);

    groupBox = new QGroupBox(CP2Scope);
    groupBox->setObjectName(QString::fromUtf8("groupBox"));
    vboxLayout3 = new QVBoxLayout(groupBox);
    vboxLayout3->setSpacing(6);
    vboxLayout3->setMargin(9);
    vboxLayout3->setObjectName(QString::fromUtf8("vboxLayout3"));
    gridLayout = new QGridLayout();
    gridLayout->setSpacing(6);
    gridLayout->setMargin(0);
    gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
    _chan2led = new QLabel(groupBox);
    _chan2led->setObjectName(QString::fromUtf8("_chan2led"));
    QSizePolicy sizePolicy9(static_cast<QSizePolicy::Policy>(1), static_cast<QSizePolicy::Policy>(5));
    sizePolicy9.setHorizontalStretch(0);
    sizePolicy9.setVerticalStretch(0);
    sizePolicy9.setHeightForWidth(_chan2led->sizePolicy().hasHeightForWidth());
    _chan2led->setSizePolicy(sizePolicy9);
    _chan2led->setMinimumSize(QSize(16, 16));
    _chan2led->setMaximumSize(QSize(16, 16));
    _chan2led->setFocusPolicy(Qt::NoFocus);
    _chan2led->setFrameShape(QFrame::Panel);
    _chan2led->setFrameShadow(QFrame::Raised);
    _chan2led->setWordWrap(false);

    gridLayout->addWidget(_chan2led, 3, 4, 1, 1);

    _chan2pulseCount = new QLabel(groupBox);
    _chan2pulseCount->setObjectName(QString::fromUtf8("_chan2pulseCount"));
    QSizePolicy sizePolicy10(static_cast<QSizePolicy::Policy>(1), static_cast<QSizePolicy::Policy>(5));
    sizePolicy10.setHorizontalStretch(0);
    sizePolicy10.setVerticalStretch(0);
    sizePolicy10.setHeightForWidth(_chan2pulseCount->sizePolicy().hasHeightForWidth());
    _chan2pulseCount->setSizePolicy(sizePolicy10);
    _chan2pulseCount->setAlignment(Qt::AlignCenter);
    _chan2pulseCount->setWordWrap(false);

    gridLayout->addWidget(_chan2pulseCount, 3, 1, 1, 1);

    _chan1errors = new QLabel(groupBox);
    _chan1errors->setObjectName(QString::fromUtf8("_chan1errors"));
    QSizePolicy sizePolicy11(static_cast<QSizePolicy::Policy>(1), static_cast<QSizePolicy::Policy>(5));
    sizePolicy11.setHorizontalStretch(0);
    sizePolicy11.setVerticalStretch(0);
    sizePolicy11.setHeightForWidth(_chan1errors->sizePolicy().hasHeightForWidth());
    _chan1errors->setSizePolicy(sizePolicy11);
    _chan1errors->setAlignment(Qt::AlignCenter);
    _chan1errors->setWordWrap(false);

    gridLayout->addWidget(_chan1errors, 2, 3, 1, 1);

    textLabel1_5 = new QLabel(groupBox);
    textLabel1_5->setObjectName(QString::fromUtf8("textLabel1_5"));
    QSizePolicy sizePolicy12(static_cast<QSizePolicy::Policy>(1), static_cast<QSizePolicy::Policy>(5));
    sizePolicy12.setHorizontalStretch(0);
    sizePolicy12.setVerticalStretch(0);
    sizePolicy12.setHeightForWidth(textLabel1_5->sizePolicy().hasHeightForWidth());
    textLabel1_5->setSizePolicy(sizePolicy12);
    textLabel1_5->setAlignment(Qt::AlignCenter);
    textLabel1_5->setWordWrap(false);

    gridLayout->addWidget(textLabel1_5, 1, 0, 1, 1);

    _chan0errors = new QLabel(groupBox);
    _chan0errors->setObjectName(QString::fromUtf8("_chan0errors"));
    QSizePolicy sizePolicy13(static_cast<QSizePolicy::Policy>(1), static_cast<QSizePolicy::Policy>(5));
    sizePolicy13.setHorizontalStretch(0);
    sizePolicy13.setVerticalStretch(0);
    sizePolicy13.setHeightForWidth(_chan0errors->sizePolicy().hasHeightForWidth());
    _chan0errors->setSizePolicy(sizePolicy13);
    _chan0errors->setAlignment(Qt::AlignCenter);
    _chan0errors->setWordWrap(false);

    gridLayout->addWidget(_chan0errors, 1, 3, 1, 1);

    _chan1pulseCount = new QLabel(groupBox);
    _chan1pulseCount->setObjectName(QString::fromUtf8("_chan1pulseCount"));
    QSizePolicy sizePolicy14(static_cast<QSizePolicy::Policy>(1), static_cast<QSizePolicy::Policy>(5));
    sizePolicy14.setHorizontalStretch(0);
    sizePolicy14.setVerticalStretch(0);
    sizePolicy14.setHeightForWidth(_chan1pulseCount->sizePolicy().hasHeightForWidth());
    _chan1pulseCount->setSizePolicy(sizePolicy14);
    _chan1pulseCount->setAlignment(Qt::AlignCenter);
    _chan1pulseCount->setWordWrap(false);

    gridLayout->addWidget(_chan1pulseCount, 2, 1, 1, 1);

    textLabel1_5_3 = new QLabel(groupBox);
    textLabel1_5_3->setObjectName(QString::fromUtf8("textLabel1_5_3"));
    QSizePolicy sizePolicy15(static_cast<QSizePolicy::Policy>(1), static_cast<QSizePolicy::Policy>(5));
    sizePolicy15.setHorizontalStretch(0);
    sizePolicy15.setVerticalStretch(0);
    sizePolicy15.setHeightForWidth(textLabel1_5_3->sizePolicy().hasHeightForWidth());
    textLabel1_5_3->setSizePolicy(sizePolicy15);
    textLabel1_5_3->setAlignment(Qt::AlignCenter);
    textLabel1_5_3->setWordWrap(false);

    gridLayout->addWidget(textLabel1_5_3, 3, 0, 1, 1);

    textLabel3_3 = new QLabel(groupBox);
    textLabel3_3->setObjectName(QString::fromUtf8("textLabel3_3"));
    QSizePolicy sizePolicy16(static_cast<QSizePolicy::Policy>(1), static_cast<QSizePolicy::Policy>(5));
    sizePolicy16.setHorizontalStretch(0);
    sizePolicy16.setVerticalStretch(0);
    sizePolicy16.setHeightForWidth(textLabel3_3->sizePolicy().hasHeightForWidth());
    textLabel3_3->setSizePolicy(sizePolicy16);
    textLabel3_3->setAlignment(Qt::AlignCenter);
    textLabel3_3->setWordWrap(false);

    gridLayout->addWidget(textLabel3_3, 0, 3, 1, 1);

    _chan0pulseCount = new QLabel(groupBox);
    _chan0pulseCount->setObjectName(QString::fromUtf8("_chan0pulseCount"));
    QSizePolicy sizePolicy17(static_cast<QSizePolicy::Policy>(1), static_cast<QSizePolicy::Policy>(5));
    sizePolicy17.setHorizontalStretch(0);
    sizePolicy17.setVerticalStretch(0);
    sizePolicy17.setHeightForWidth(_chan0pulseCount->sizePolicy().hasHeightForWidth());
    _chan0pulseCount->setSizePolicy(sizePolicy17);
    _chan0pulseCount->setAlignment(Qt::AlignCenter);
    _chan0pulseCount->setWordWrap(false);

    gridLayout->addWidget(_chan0pulseCount, 1, 1, 1, 1);

    _chan0led = new QLabel(groupBox);
    _chan0led->setObjectName(QString::fromUtf8("_chan0led"));
    QSizePolicy sizePolicy18(static_cast<QSizePolicy::Policy>(1), static_cast<QSizePolicy::Policy>(5));
    sizePolicy18.setHorizontalStretch(0);
    sizePolicy18.setVerticalStretch(0);
    sizePolicy18.setHeightForWidth(_chan0led->sizePolicy().hasHeightForWidth());
    _chan0led->setSizePolicy(sizePolicy18);
    _chan0led->setMinimumSize(QSize(16, 16));
    _chan0led->setMaximumSize(QSize(16, 16));
    _chan0led->setFocusPolicy(Qt::NoFocus);
    _chan0led->setFrameShape(QFrame::Panel);
    _chan0led->setFrameShadow(QFrame::Raised);
    _chan0led->setWordWrap(false);

    gridLayout->addWidget(_chan0led, 1, 4, 1, 1);

    textLabel1 = new QLabel(groupBox);
    textLabel1->setObjectName(QString::fromUtf8("textLabel1"));
    QSizePolicy sizePolicy19(static_cast<QSizePolicy::Policy>(1), static_cast<QSizePolicy::Policy>(5));
    sizePolicy19.setHorizontalStretch(0);
    sizePolicy19.setVerticalStretch(0);
    sizePolicy19.setHeightForWidth(textLabel1->sizePolicy().hasHeightForWidth());
    textLabel1->setSizePolicy(sizePolicy19);
    textLabel1->setWordWrap(false);

    gridLayout->addWidget(textLabel1, 0, 4, 1, 1);

    _chan2errors = new QLabel(groupBox);
    _chan2errors->setObjectName(QString::fromUtf8("_chan2errors"));
    QSizePolicy sizePolicy20(static_cast<QSizePolicy::Policy>(1), static_cast<QSizePolicy::Policy>(5));
    sizePolicy20.setHorizontalStretch(0);
    sizePolicy20.setVerticalStretch(0);
    sizePolicy20.setHeightForWidth(_chan2errors->sizePolicy().hasHeightForWidth());
    _chan2errors->setSizePolicy(sizePolicy20);
    _chan2errors->setAlignment(Qt::AlignCenter);
    _chan2errors->setWordWrap(false);

    gridLayout->addWidget(_chan2errors, 3, 3, 1, 1);

    textLabel4 = new QLabel(groupBox);
    textLabel4->setObjectName(QString::fromUtf8("textLabel4"));
    QSizePolicy sizePolicy21(static_cast<QSizePolicy::Policy>(1), static_cast<QSizePolicy::Policy>(5));
    sizePolicy21.setHorizontalStretch(0);
    sizePolicy21.setVerticalStretch(0);
    sizePolicy21.setHeightForWidth(textLabel4->sizePolicy().hasHeightForWidth());
    textLabel4->setSizePolicy(sizePolicy21);
    textLabel4->setAlignment(Qt::AlignCenter);
    textLabel4->setWordWrap(false);

    gridLayout->addWidget(textLabel4, 0, 0, 1, 1);

    textLabel1_5_2 = new QLabel(groupBox);
    textLabel1_5_2->setObjectName(QString::fromUtf8("textLabel1_5_2"));
    QSizePolicy sizePolicy22(static_cast<QSizePolicy::Policy>(1), static_cast<QSizePolicy::Policy>(5));
    sizePolicy22.setHorizontalStretch(0);
    sizePolicy22.setVerticalStretch(0);
    sizePolicy22.setHeightForWidth(textLabel1_5_2->sizePolicy().hasHeightForWidth());
    textLabel1_5_2->setSizePolicy(sizePolicy22);
    textLabel1_5_2->setAlignment(Qt::AlignCenter);
    textLabel1_5_2->setWordWrap(false);

    gridLayout->addWidget(textLabel1_5_2, 2, 0, 1, 1);

    textLabel3_2 = new QLabel(groupBox);
    textLabel3_2->setObjectName(QString::fromUtf8("textLabel3_2"));
    QSizePolicy sizePolicy23(static_cast<QSizePolicy::Policy>(1), static_cast<QSizePolicy::Policy>(5));
    sizePolicy23.setHorizontalStretch(0);
    sizePolicy23.setVerticalStretch(0);
    sizePolicy23.setHeightForWidth(textLabel3_2->sizePolicy().hasHeightForWidth());
    textLabel3_2->setSizePolicy(sizePolicy23);
    textLabel3_2->setAlignment(Qt::AlignCenter);
    textLabel3_2->setWordWrap(false);

    gridLayout->addWidget(textLabel3_2, 0, 2, 1, 1);

    _chan2pulseRate = new QLabel(groupBox);
    _chan2pulseRate->setObjectName(QString::fromUtf8("_chan2pulseRate"));
    QSizePolicy sizePolicy24(static_cast<QSizePolicy::Policy>(1), static_cast<QSizePolicy::Policy>(5));
    sizePolicy24.setHorizontalStretch(0);
    sizePolicy24.setVerticalStretch(0);
    sizePolicy24.setHeightForWidth(_chan2pulseRate->sizePolicy().hasHeightForWidth());
    _chan2pulseRate->setSizePolicy(sizePolicy24);
    _chan2pulseRate->setAlignment(Qt::AlignCenter);
    _chan2pulseRate->setWordWrap(false);

    gridLayout->addWidget(_chan2pulseRate, 3, 2, 1, 1);

    _chan0pulseRate = new QLabel(groupBox);
    _chan0pulseRate->setObjectName(QString::fromUtf8("_chan0pulseRate"));
    QSizePolicy sizePolicy25(static_cast<QSizePolicy::Policy>(1), static_cast<QSizePolicy::Policy>(5));
    sizePolicy25.setHorizontalStretch(0);
    sizePolicy25.setVerticalStretch(0);
    sizePolicy25.setHeightForWidth(_chan0pulseRate->sizePolicy().hasHeightForWidth());
    _chan0pulseRate->setSizePolicy(sizePolicy25);
    _chan0pulseRate->setAlignment(Qt::AlignCenter);
    _chan0pulseRate->setWordWrap(false);

    gridLayout->addWidget(_chan0pulseRate, 1, 2, 1, 1);

    _chan1led = new QLabel(groupBox);
    _chan1led->setObjectName(QString::fromUtf8("_chan1led"));
    QSizePolicy sizePolicy26(static_cast<QSizePolicy::Policy>(1), static_cast<QSizePolicy::Policy>(5));
    sizePolicy26.setHorizontalStretch(0);
    sizePolicy26.setVerticalStretch(0);
    sizePolicy26.setHeightForWidth(_chan1led->sizePolicy().hasHeightForWidth());
    _chan1led->setSizePolicy(sizePolicy26);
    _chan1led->setMinimumSize(QSize(16, 16));
    _chan1led->setMaximumSize(QSize(16, 16));
    _chan1led->setFocusPolicy(Qt::NoFocus);
    _chan1led->setFrameShape(QFrame::Panel);
    _chan1led->setFrameShadow(QFrame::Raised);
    _chan1led->setWordWrap(false);

    gridLayout->addWidget(_chan1led, 2, 4, 1, 1);

    textLabel3 = new QLabel(groupBox);
    textLabel3->setObjectName(QString::fromUtf8("textLabel3"));
    QSizePolicy sizePolicy27(static_cast<QSizePolicy::Policy>(1), static_cast<QSizePolicy::Policy>(5));
    sizePolicy27.setHorizontalStretch(0);
    sizePolicy27.setVerticalStretch(0);
    sizePolicy27.setHeightForWidth(textLabel3->sizePolicy().hasHeightForWidth());
    textLabel3->setSizePolicy(sizePolicy27);
    textLabel3->setAlignment(Qt::AlignCenter);
    textLabel3->setWordWrap(false);

    gridLayout->addWidget(textLabel3, 0, 1, 1, 1);

    _chan1pulseRate = new QLabel(groupBox);
    _chan1pulseRate->setObjectName(QString::fromUtf8("_chan1pulseRate"));
    QSizePolicy sizePolicy28(static_cast<QSizePolicy::Policy>(1), static_cast<QSizePolicy::Policy>(5));
    sizePolicy28.setHorizontalStretch(0);
    sizePolicy28.setVerticalStretch(0);
    sizePolicy28.setHeightForWidth(_chan1pulseRate->sizePolicy().hasHeightForWidth());
    _chan1pulseRate->setSizePolicy(sizePolicy28);
    _chan1pulseRate->setAlignment(Qt::AlignCenter);
    _chan1pulseRate->setWordWrap(false);

    gridLayout->addWidget(_chan1pulseRate, 2, 2, 1, 1);


    vboxLayout3->addLayout(gridLayout);


    vboxLayout->addWidget(groupBox);

    gridLayout1 = new QGridLayout();
    gridLayout1->setSpacing(6);
    gridLayout1->setMargin(0);
    gridLayout1->setObjectName(QString::fromUtf8("gridLayout1"));
    hboxLayout4 = new QHBoxLayout();
    hboxLayout4->setSpacing(6);
    hboxLayout4->setMargin(0);
    hboxLayout4->setObjectName(QString::fromUtf8("hboxLayout4"));
    m_pTextIPname_2 = new QLabel(CP2Scope);
    m_pTextIPname_2->setObjectName(QString::fromUtf8("m_pTextIPname_2"));
    m_pTextIPname_2->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
    m_pTextIPname_2->setWordWrap(false);

    hboxLayout4->addWidget(m_pTextIPname_2);

    m_pulseIP = new QLabel(CP2Scope);
    m_pulseIP->setObjectName(QString::fromUtf8("m_pulseIP"));
    QSizePolicy sizePolicy29(static_cast<QSizePolicy::Policy>(1), static_cast<QSizePolicy::Policy>(5));
    sizePolicy29.setHorizontalStretch(0);
    sizePolicy29.setVerticalStretch(0);
    sizePolicy29.setHeightForWidth(m_pulseIP->sizePolicy().hasHeightForWidth());
    m_pulseIP->setSizePolicy(sizePolicy29);
    m_pulseIP->setWordWrap(false);

    hboxLayout4->addWidget(m_pulseIP);


    gridLayout1->addLayout(hboxLayout4, 0, 0, 1, 1);

    hboxLayout5 = new QHBoxLayout();
    hboxLayout5->setSpacing(6);
    hboxLayout5->setMargin(0);
    hboxLayout5->setObjectName(QString::fromUtf8("hboxLayout5"));
    m_pTextIPname_4 = new QLabel(CP2Scope);
    m_pTextIPname_4->setObjectName(QString::fromUtf8("m_pTextIPname_4"));
    m_pTextIPname_4->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
    m_pTextIPname_4->setWordWrap(false);

    hboxLayout5->addWidget(m_pTextIPname_4);

    m_pulseDec = new QLabel(CP2Scope);
    m_pulseDec->setObjectName(QString::fromUtf8("m_pulseDec"));
    QSizePolicy sizePolicy30(static_cast<QSizePolicy::Policy>(1), static_cast<QSizePolicy::Policy>(5));
    sizePolicy30.setHorizontalStretch(0);
    sizePolicy30.setVerticalStretch(0);
    sizePolicy30.setHeightForWidth(m_pulseDec->sizePolicy().hasHeightForWidth());
    m_pulseDec->setSizePolicy(sizePolicy30);
    m_pulseDec->setWordWrap(false);

    hboxLayout5->addWidget(m_pulseDec);


    gridLayout1->addLayout(hboxLayout5, 0, 1, 1, 1);

    hboxLayout6 = new QHBoxLayout();
    hboxLayout6->setSpacing(6);
    hboxLayout6->setMargin(0);
    hboxLayout6->setObjectName(QString::fromUtf8("hboxLayout6"));
    m_pTextIPname_3 = new QLabel(CP2Scope);
    m_pTextIPname_3->setObjectName(QString::fromUtf8("m_pTextIPname_3"));
    m_pTextIPname_3->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
    m_pTextIPname_3->setWordWrap(false);

    hboxLayout6->addWidget(m_pTextIPname_3);

    m_productDec = new QLabel(CP2Scope);
    m_productDec->setObjectName(QString::fromUtf8("m_productDec"));
    QSizePolicy sizePolicy31(static_cast<QSizePolicy::Policy>(1), static_cast<QSizePolicy::Policy>(5));
    sizePolicy31.setHorizontalStretch(0);
    sizePolicy31.setVerticalStretch(0);
    sizePolicy31.setHeightForWidth(m_productDec->sizePolicy().hasHeightForWidth());
    m_productDec->setSizePolicy(sizePolicy31);
    m_productDec->setWordWrap(false);

    hboxLayout6->addWidget(m_productDec);


    gridLayout1->addLayout(hboxLayout6, 1, 1, 1, 1);

    hboxLayout7 = new QHBoxLayout();
    hboxLayout7->setSpacing(6);
    hboxLayout7->setMargin(0);
    hboxLayout7->setObjectName(QString::fromUtf8("hboxLayout7"));
    m_pTextIPname = new QLabel(CP2Scope);
    m_pTextIPname->setObjectName(QString::fromUtf8("m_pTextIPname"));
    m_pTextIPname->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
    m_pTextIPname->setWordWrap(false);

    hboxLayout7->addWidget(m_pTextIPname);

    m_productIP = new QLabel(CP2Scope);
    m_productIP->setObjectName(QString::fromUtf8("m_productIP"));
    QSizePolicy sizePolicy32(static_cast<QSizePolicy::Policy>(1), static_cast<QSizePolicy::Policy>(5));
    sizePolicy32.setHorizontalStretch(0);
    sizePolicy32.setVerticalStretch(0);
    sizePolicy32.setHeightForWidth(m_productIP->sizePolicy().hasHeightForWidth());
    m_productIP->setSizePolicy(sizePolicy32);
    m_productIP->setWordWrap(false);

    hboxLayout7->addWidget(m_productIP);


    gridLayout1->addLayout(hboxLayout7, 1, 0, 1, 1);


    vboxLayout->addLayout(gridLayout1);


    hboxLayout->addLayout(vboxLayout);


    retranslateUi(CP2Scope);

    QSize size(1093, 747);
    size = size.expandedTo(CP2Scope->minimumSizeHint());
    CP2Scope->resize(size);


    QMetaObject::connectSlotsByName(CP2Scope);
    } // setupUi

    void retranslateUi(QWidget *CP2Scope)
    {
    CP2Scope->setWindowTitle(QApplication::translate("CP2Scope", "CP2Scope", 0, QApplication::UnicodeUTF8));
    _typeTab->setTabText(_typeTab->indexOf(Channels), QApplication::translate("CP2Scope", "Chan", 0, QApplication::UnicodeUTF8));
    pauseButton->setToolTip(QApplication::translate("CP2Scope", "pause the display", 0, QApplication::UnicodeUTF8));
    pauseButton->setText(QApplication::translate("CP2Scope", "Pause", 0, QApplication::UnicodeUTF8));
    xGrid->setText(QApplication::translate("CP2Scope", "X Grid", 0, QApplication::UnicodeUTF8));
    yGrid->setText(QApplication::translate("CP2Scope", "Y Grid", 0, QApplication::UnicodeUTF8));
    _autoScale->setText(QApplication::translate("CP2Scope", "Auto Scale", 0, QApplication::UnicodeUTF8));
    _saveImage->setText(QApplication::translate("CP2Scope", "Save Image", 0, QApplication::UnicodeUTF8));
    _up->setText(QApplication::translate("CP2Scope", "^", 0, QApplication::UnicodeUTF8));
    _dn->setText(QApplication::translate("CP2Scope", "v", 0, QApplication::UnicodeUTF8));
    groupBox->setTitle(QApplication::translate("CP2Scope", "Pulse Statistics", 0, QApplication::UnicodeUTF8));
    _chan2led->setText(QString());
    _chan2pulseCount->setText(QApplication::translate("CP2Scope", "0", 0, QApplication::UnicodeUTF8));
    _chan1errors->setText(QApplication::translate("CP2Scope", "0", 0, QApplication::UnicodeUTF8));
    textLabel1_5->setText(QApplication::translate("CP2Scope", "S", 0, QApplication::UnicodeUTF8));
    _chan0errors->setText(QApplication::translate("CP2Scope", "0", 0, QApplication::UnicodeUTF8));
    _chan1pulseCount->setText(QApplication::translate("CP2Scope", "0", 0, QApplication::UnicodeUTF8));
    textLabel1_5_3->setText(QApplication::translate("CP2Scope", "Xv", 0, QApplication::UnicodeUTF8));
    textLabel3_3->setText(QApplication::translate("CP2Scope", "Errors", 0, QApplication::UnicodeUTF8));
    _chan0pulseCount->setText(QApplication::translate("CP2Scope", "0", 0, QApplication::UnicodeUTF8));
    _chan0led->setText(QString());
    textLabel1->setText(QApplication::translate("CP2Scope", "EOF", 0, QApplication::UnicodeUTF8));
    _chan2errors->setText(QApplication::translate("CP2Scope", "0", 0, QApplication::UnicodeUTF8));
    textLabel4->setText(QApplication::translate("CP2Scope", "Channel", 0, QApplication::UnicodeUTF8));
    textLabel1_5_2->setText(QApplication::translate("CP2Scope", "Xh", 0, QApplication::UnicodeUTF8));
    textLabel3_2->setText(QApplication::translate("CP2Scope", "Rate (/s)", 0, QApplication::UnicodeUTF8));
    _chan2pulseRate->setText(QApplication::translate("CP2Scope", "0", 0, QApplication::UnicodeUTF8));
    _chan0pulseRate->setText(QApplication::translate("CP2Scope", "0", 0, QApplication::UnicodeUTF8));
    _chan1led->setText(QString());
    textLabel3->setText(QApplication::translate("CP2Scope", "Number (K)", 0, QApplication::UnicodeUTF8));
    _chan1pulseRate->setText(QApplication::translate("CP2Scope", "0", 0, QApplication::UnicodeUTF8));
    m_pTextIPname_2->setText(QApplication::translate("CP2Scope", "Pulse IP", 0, QApplication::UnicodeUTF8));
    m_pulseIP->setText(QApplication::translate("CP2Scope", "xxx.xxx.xxx.xxx", 0, QApplication::UnicodeUTF8));
    m_pTextIPname_4->setText(QApplication::translate("CP2Scope", "Decimation", 0, QApplication::UnicodeUTF8));
    m_pulseDec->setText(QApplication::translate("CP2Scope", "1", 0, QApplication::UnicodeUTF8));
    m_pTextIPname_3->setText(QApplication::translate("CP2Scope", "Decimation", 0, QApplication::UnicodeUTF8));
    m_productDec->setText(QApplication::translate("CP2Scope", "1", 0, QApplication::UnicodeUTF8));
    m_pTextIPname->setText(QApplication::translate("CP2Scope", "Product IP", 0, QApplication::UnicodeUTF8));
    m_productIP->setText(QApplication::translate("CP2Scope", "xxx.xxx.xxx.xxx", 0, QApplication::UnicodeUTF8));
    Q_UNUSED(CP2Scope);
    } // retranslateUi

};

namespace Ui {
    class CP2Scope: public Ui_CP2Scope {};
} // namespace Ui

#endif // UI_CP2SCOPE_H

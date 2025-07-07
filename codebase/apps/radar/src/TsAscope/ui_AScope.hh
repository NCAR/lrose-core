/********************************************************************************
 ** Form generated from reading UI file 'AScope.ui'
 **
 ** Created by: Qt User Interface Compiler version 4.8.7
 **
 ** WARNING! All changes made in this file will be lost when recompiling UI file!
 ********************************************************************************/

#ifndef UI_ASCOPE_H
#define UI_ASCOPE_H

#include <QVariant>
#include <QAction>
#include <QApplication>
#include <QButtonGroup>
#include <QCheckBox>
#include <QComboBox>
#include <QFormLayout>
#include <QFrame>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QProgressBar>
#include <QPushButton>
#include <QSpacerItem>
#include <QTabWidget>
#include <QToolButton>
#include <QVBoxLayout>
#include <QWidget>
#include "Knob.hh"
#include "ScopePlot.hh"

QT_BEGIN_NAMESPACE

  class Ui_AScope
{
public:
  QHBoxLayout *horizontalLayout_2;
  ScopePlot *_scopePlot;
  QVBoxLayout *verticalLayout_2;
  QHBoxLayout *hboxLayout;
  QGroupBox *_chanBox;
  QGroupBox *groupBox_5;
  QHBoxLayout *hboxLayout1;
  QTabWidget *_typeTab;
  QWidget *Channels;
  QHBoxLayout *horizontalLayout;
  QVBoxLayout *verticalLayout;
  QGroupBox *groupBox_2;
  QFormLayout *formLayout;
  QLabel *label;
  QLineEdit *_gateNumEditor;
  QLabel *_nGatesLabel;
  QLabel *label_2;
  QComboBox *_blockSizeCombo;
  QCheckBox *_alongBeamCheck;
  QHBoxLayout *hboxLayout2;
  QLabel *label_3;
  QLabel *_powerDB;
  QVBoxLayout *vboxLayout;
  QCheckBox *_windowButton;
  QCheckBox *_pauseButton;
  QCheckBox *_xGrid;
  QCheckBox *_yGrid;
  QHBoxLayout *hboxLayout3;
  QPushButton *_autoScale;
  QPushButton *_saveImage;
  QHBoxLayout *hboxLayout4;
  QVBoxLayout *vboxLayout1;
  QToolButton *_up;
  QToolButton *_dn;
  Knob *_gainKnob;
  QFrame *_userFrame;
  QSpacerItem *verticalSpacer;
  QProgressBar *_activityBar;

  void setupUi(QWidget *AScope)
  {
    if (AScope->objectName().isEmpty())
      AScope->setObjectName(QString::fromUtf8("AScope"));
    AScope->resize(930, 618);
    horizontalLayout_2 = new QHBoxLayout(AScope);
    horizontalLayout_2->setObjectName(QString::fromUtf8("horizontalLayout_2"));
    _scopePlot = new ScopePlot(AScope);
    _scopePlot->setObjectName(QString::fromUtf8("_scopePlot"));
    QSizePolicy sizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    sizePolicy.setHorizontalStretch(0);
    sizePolicy.setVerticalStretch(0);
    sizePolicy.setHeightForWidth(_scopePlot->sizePolicy().hasHeightForWidth());
    _scopePlot->setSizePolicy(sizePolicy);
    _scopePlot->setMinimumSize(QSize(600, 600));

    horizontalLayout_2->addWidget(_scopePlot);

    verticalLayout_2 = new QVBoxLayout();
    verticalLayout_2->setObjectName(QString::fromUtf8("verticalLayout_2"));
    hboxLayout = new QHBoxLayout();
#ifndef Q_OS_MAC
    hboxLayout->setSpacing(6);
#endif
#ifndef Q_OS_MAC
    hboxLayout->setContentsMargins(0, 0, 0, 0);
#endif
    hboxLayout->setObjectName(QString::fromUtf8("hboxLayout"));
    _chanBox = new QGroupBox(AScope);
    _chanBox->setObjectName(QString::fromUtf8("_chanBox"));
    QSizePolicy sizePolicy1(QSizePolicy::Minimum, QSizePolicy::Minimum);
    sizePolicy1.setHorizontalStretch(0);
    sizePolicy1.setVerticalStretch(0);
    sizePolicy1.setHeightForWidth(_chanBox->sizePolicy().hasHeightForWidth());
    _chanBox->setSizePolicy(sizePolicy1);
    _chanBox->setMinimumSize(QSize(0, 0));

    hboxLayout->addWidget(_chanBox);

    groupBox_5 = new QGroupBox(AScope);
    groupBox_5->setObjectName(QString::fromUtf8("groupBox_5"));
    QSizePolicy sizePolicy2(QSizePolicy::Minimum, QSizePolicy::Preferred);
    sizePolicy2.setHorizontalStretch(0);
    sizePolicy2.setVerticalStretch(0);
    sizePolicy2.setHeightForWidth(groupBox_5->sizePolicy().hasHeightForWidth());
    groupBox_5->setSizePolicy(sizePolicy2);
    hboxLayout1 = new QHBoxLayout(groupBox_5);
#ifndef Q_OS_MAC
    hboxLayout1->setSpacing(6);
#endif
#ifndef Q_OS_MAC
    hboxLayout1->setContentsMargins(9, 9, 9, 9);
#endif
    hboxLayout1->setObjectName(QString::fromUtf8("hboxLayout1"));
    _typeTab = new QTabWidget(groupBox_5);
    _typeTab->setObjectName(QString::fromUtf8("_typeTab"));
    sizePolicy1.setHeightForWidth(_typeTab->sizePolicy().hasHeightForWidth());
    _typeTab->setSizePolicy(sizePolicy1);
    _typeTab->setMinimumSize(QSize(200, 200));
    _typeTab->setMaximumSize(QSize(16777215, 16777215));
    Channels = new QWidget();
    Channels->setObjectName(QString::fromUtf8("Channels"));
    _typeTab->addTab(Channels, QString());

    hboxLayout1->addWidget(_typeTab);


    hboxLayout->addWidget(groupBox_5);


    verticalLayout_2->addLayout(hboxLayout);

    horizontalLayout = new QHBoxLayout();
    horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
    verticalLayout = new QVBoxLayout();
    verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
    groupBox_2 = new QGroupBox(AScope);
    groupBox_2->setObjectName(QString::fromUtf8("groupBox_2"));
    QSizePolicy sizePolicy3(QSizePolicy::Minimum, QSizePolicy::MinimumExpanding);
    sizePolicy3.setHorizontalStretch(0);
    sizePolicy3.setVerticalStretch(0);
    sizePolicy3.setHeightForWidth(groupBox_2->sizePolicy().hasHeightForWidth());
    groupBox_2->setSizePolicy(sizePolicy3);
    formLayout = new QFormLayout(groupBox_2);
    formLayout->setObjectName(QString::fromUtf8("formLayout"));
    label = new QLabel(groupBox_2);
    label->setObjectName(QString::fromUtf8("label"));
    sizePolicy2.setHeightForWidth(label->sizePolicy().hasHeightForWidth());
    label->setSizePolicy(sizePolicy2);
    label->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

    formLayout->setWidget(0, QFormLayout::LabelRole, label);

    _gateNumEditor = new QLineEdit(groupBox_2);
    _gateNumEditor->setObjectName(QString::fromUtf8("_gateNumEditor"));
    QSizePolicy sizePolicy4(QSizePolicy::Minimum, QSizePolicy::Fixed);
    sizePolicy4.setHorizontalStretch(0);
    sizePolicy4.setVerticalStretch(0);
    sizePolicy4.setHeightForWidth(_gateNumEditor->sizePolicy().hasHeightForWidth());
    _gateNumEditor->setSizePolicy(sizePolicy4);

    formLayout->setWidget(0, QFormLayout::FieldRole, _gateNumEditor);

    label_2 = new QLabel(groupBox_2);
    label_2->setObjectName(QString::fromUtf8("label_2"));
    sizePolicy2.setHeightForWidth(label_2->sizePolicy().hasHeightForWidth());
    label_2->setSizePolicy(sizePolicy2);
    label_2->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

    formLayout->setWidget(1, QFormLayout::LabelRole, label_2);

    _blockSizeCombo = new QComboBox(groupBox_2);
    _blockSizeCombo->setObjectName(QString::fromUtf8("_blockSizeCombo"));
    sizePolicy4.setHeightForWidth(_blockSizeCombo->sizePolicy().hasHeightForWidth());
    _blockSizeCombo->setSizePolicy(sizePolicy4);

    formLayout->setWidget(1, QFormLayout::FieldRole, _blockSizeCombo);

    _alongBeamCheck = new QCheckBox(groupBox_2);
    _alongBeamCheck->setObjectName(QString::fromUtf8("_alongBeamCheck"));

    formLayout->setWidget(2, QFormLayout::LabelRole, _alongBeamCheck);

    _nGatesLabel = new QLabel(groupBox_2);
    _nGatesLabel->setObjectName(QString::fromUtf8("_nGatesLabel"));
    _nGatesLabel->setText("NumGates: 0");
    formLayout->setWidget(2, QFormLayout::FieldRole, _nGatesLabel);

    verticalLayout->addWidget(groupBox_2);

    hboxLayout2 = new QHBoxLayout();
#ifndef Q_OS_MAC
    hboxLayout2->setSpacing(6);
#endif
    hboxLayout2->setContentsMargins(0, 0, 0, 0);
    hboxLayout2->setObjectName(QString::fromUtf8("hboxLayout2"));
    label_3 = new QLabel(AScope);
    label_3->setObjectName(QString::fromUtf8("label_3"));
    sizePolicy1.setHeightForWidth(label_3->sizePolicy().hasHeightForWidth());
    label_3->setSizePolicy(sizePolicy1);
    label_3->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

    hboxLayout2->addWidget(label_3);

    _powerDB = new QLabel(AScope);
    _powerDB->setObjectName(QString::fromUtf8("_powerDB"));
    sizePolicy2.setHeightForWidth(_powerDB->sizePolicy().hasHeightForWidth());
    _powerDB->setSizePolicy(sizePolicy2);
    _powerDB->setAlignment(Qt::AlignCenter);

    hboxLayout2->addWidget(_powerDB);


    verticalLayout->addLayout(hboxLayout2);


    horizontalLayout->addLayout(verticalLayout);

    vboxLayout = new QVBoxLayout();
#ifndef Q_OS_MAC
    vboxLayout->setSpacing(6);
#endif
    vboxLayout->setContentsMargins(0, 0, 0, 0);
    vboxLayout->setObjectName(QString::fromUtf8("vboxLayout"));
    _windowButton = new QCheckBox(AScope);
    _windowButton->setObjectName(QString::fromUtf8("_windowButton"));
    sizePolicy1.setHeightForWidth(_windowButton->sizePolicy().hasHeightForWidth());
    _windowButton->setSizePolicy(sizePolicy1);

    vboxLayout->addWidget(_windowButton);

    _pauseButton = new QCheckBox(AScope);
    _pauseButton->setObjectName(QString::fromUtf8("_pauseButton"));
    sizePolicy1.setHeightForWidth(_pauseButton->sizePolicy().hasHeightForWidth());
    _pauseButton->setSizePolicy(sizePolicy1);

    vboxLayout->addWidget(_pauseButton);

    _xGrid = new QCheckBox(AScope);
    _xGrid->setObjectName(QString::fromUtf8("_xGrid"));
    sizePolicy2.setHeightForWidth(_xGrid->sizePolicy().hasHeightForWidth());
    _xGrid->setSizePolicy(sizePolicy2);
    _xGrid->setChecked(true);

    vboxLayout->addWidget(_xGrid);

    _yGrid = new QCheckBox(AScope);
    _yGrid->setObjectName(QString::fromUtf8("_yGrid"));
    sizePolicy2.setHeightForWidth(_yGrid->sizePolicy().hasHeightForWidth());
    _yGrid->setSizePolicy(sizePolicy2);
    _yGrid->setChecked(true);

    vboxLayout->addWidget(_yGrid);


    horizontalLayout->addLayout(vboxLayout);


    verticalLayout_2->addLayout(horizontalLayout);

    hboxLayout3 = new QHBoxLayout();
#ifndef Q_OS_MAC
    hboxLayout3->setSpacing(6);
#endif
    hboxLayout3->setContentsMargins(0, 0, 0, 0);
    hboxLayout3->setObjectName(QString::fromUtf8("hboxLayout3"));
    _autoScale = new QPushButton(AScope);
    _autoScale->setObjectName(QString::fromUtf8("_autoScale"));
    sizePolicy1.setHeightForWidth(_autoScale->sizePolicy().hasHeightForWidth());
    _autoScale->setSizePolicy(sizePolicy1);

    hboxLayout3->addWidget(_autoScale);

    _saveImage = new QPushButton(AScope);
    _saveImage->setObjectName(QString::fromUtf8("_saveImage"));

    hboxLayout3->addWidget(_saveImage);


    verticalLayout_2->addLayout(hboxLayout3);

    hboxLayout4 = new QHBoxLayout();
#ifndef Q_OS_MAC
    hboxLayout4->setSpacing(6);
#endif
    hboxLayout4->setContentsMargins(0, 0, 0, 0);
    hboxLayout4->setObjectName(QString::fromUtf8("hboxLayout4"));
    vboxLayout1 = new QVBoxLayout();
#ifndef Q_OS_MAC
    vboxLayout1->setSpacing(6);
#endif
#ifndef Q_OS_MAC
    vboxLayout1->setContentsMargins(0, 0, 0, 0);
#endif
    vboxLayout1->setObjectName(QString::fromUtf8("vboxLayout1"));
    _up = new QToolButton(AScope);
    _up->setObjectName(QString::fromUtf8("_up"));
    QSizePolicy sizePolicy5(QSizePolicy::Fixed, QSizePolicy::MinimumExpanding);
    sizePolicy5.setHorizontalStretch(0);
    sizePolicy5.setVerticalStretch(0);
    sizePolicy5.setHeightForWidth(_up->sizePolicy().hasHeightForWidth());
    _up->setSizePolicy(sizePolicy5);
    _up->setMinimumSize(QSize(30, 0));
    _up->setMaximumSize(QSize(20, 16777215));
    _up->setAutoRepeat(true);

    vboxLayout1->addWidget(_up);

    _dn = new QToolButton(AScope);
    _dn->setObjectName(QString::fromUtf8("_dn"));
    sizePolicy5.setHeightForWidth(_dn->sizePolicy().hasHeightForWidth());
    _dn->setSizePolicy(sizePolicy5);
    _dn->setMinimumSize(QSize(30, 0));
    _dn->setMaximumSize(QSize(30, 16777215));
    _dn->setAutoRepeat(true);

    vboxLayout1->addWidget(_dn);


    hboxLayout4->addLayout(vboxLayout1);

    _gainKnob = new Knob(AScope);
    _gainKnob->setObjectName(QString::fromUtf8("_gainKnob"));
    sizePolicy1.setHeightForWidth(_gainKnob->sizePolicy().hasHeightForWidth());
    _gainKnob->setSizePolicy(sizePolicy1);
    _gainKnob->setMinimumSize(QSize(100, 0));
    _gainKnob->setMaximumSize(QSize(200, 16777215));

    hboxLayout4->addWidget(_gainKnob);


    verticalLayout_2->addLayout(hboxLayout4);

    _userFrame = new QFrame(AScope);
    _userFrame->setObjectName(QString::fromUtf8("_userFrame"));
    _userFrame->setFrameShape(QFrame::StyledPanel);
    _userFrame->setFrameShadow(QFrame::Raised);

    verticalLayout_2->addWidget(_userFrame);

    verticalSpacer = new QSpacerItem(20, 17, QSizePolicy::Minimum, QSizePolicy::Expanding);

    verticalLayout_2->addItem(verticalSpacer);

    _activityBar = new QProgressBar(AScope);
    _activityBar->setObjectName(QString::fromUtf8("_activityBar"));
    sizePolicy4.setHeightForWidth(_activityBar->sizePolicy().hasHeightForWidth());
    _activityBar->setSizePolicy(sizePolicy4);
    QFont font;
    font.setPointSize(1);
    _activityBar->setFont(font);
    _activityBar->setValue(24);
    _activityBar->setTextVisible(false);
    _activityBar->setOrientation(Qt::Horizontal);

    verticalLayout_2->addWidget(_activityBar);


    horizontalLayout_2->addLayout(verticalLayout_2);


    retranslateUi(AScope);

    QMetaObject::connectSlotsByName(AScope);
  } // setupUi

  void retranslateUi(QWidget *AScope)
  {
    AScope->setWindowTitle(QApplication::translate("AScope", "AScope", 0));
    _chanBox->setTitle(QApplication::translate("AScope", "Channel", 0));
    groupBox_5->setTitle(QApplication::translate("AScope", "Product", 0));
#ifndef QT_NO_TOOLTIP
    _typeTab->setToolTip(QApplication::translate("AScope", "Select the displayed product.", 0));
#endif // QT_NO_TOOLTIP
    _typeTab->setTabText(_typeTab->indexOf(Channels), QApplication::translate("AScope", "Chan", 0));
    groupBox_2->setTitle(QApplication::translate("AScope", "Display Mode", 0));
    label->setText(QApplication::translate("AScope", "Gate Number", 0));
#ifndef QT_NO_TOOLTIP
    _gateNumEditor->setToolTip(QApplication::translate("AScope", "Select the gate for the one gate display.", 0));
#endif // QT_NO_TOOLTIP
    label_2->setText(QApplication::translate("AScope", "Block size", 0));
#ifndef QT_NO_TOOLTIP
    _blockSizeCombo->setToolTip(QApplication::translate("AScope", "Set the number of samples for one gate mode.", 0));
#endif // QT_NO_TOOLTIP
    _alongBeamCheck->setText(QApplication::translate("AScope", "Along Beam", 0));
    label_3->setText(QApplication::translate("AScope", "Power (DB)", 0));
    _powerDB->setText(QApplication::translate("AScope", "0", 0));
#ifndef QT_NO_TOOLTIP
    _windowButton->setToolTip(QApplication::translate("AScope", "Enable time series windowing for the power spectrum.", 0));
#endif // QT_NO_TOOLTIP
    _windowButton->setText(QApplication::translate("AScope", "Windowing", 0));
#ifndef QT_NO_TOOLTIP
    _pauseButton->setToolTip(QApplication::translate("AScope", "Pause the display.", 0));
#endif // QT_NO_TOOLTIP
    _pauseButton->setText(QApplication::translate("AScope", "Pause", 0));
#ifndef QT_NO_TOOLTIP
    _xGrid->setToolTip(QApplication::translate("AScope", "Enable X grid display.", 0));
#endif // QT_NO_TOOLTIP
    _xGrid->setText(QApplication::translate("AScope", "X Grid", 0));
#ifndef QT_NO_TOOLTIP
    _yGrid->setToolTip(QApplication::translate("AScope", "Enable Y grid display.", 0));
#endif // QT_NO_TOOLTIP
    _yGrid->setText(QApplication::translate("AScope", "Y Grid", 0));
#ifndef QT_NO_TOOLTIP
    _autoScale->setToolTip(QApplication::translate("AScope", "Auto scale the display.", 0));
#endif // QT_NO_TOOLTIP
    _autoScale->setText(QApplication::translate("AScope", "Auto Scale", 0));
#ifndef QT_NO_TOOLTIP
    _saveImage->setToolTip(QApplication::translate("AScope", "Save the display image to a file.", 0));
#endif // QT_NO_TOOLTIP
    _saveImage->setText(QApplication::translate("AScope", "Save Image", 0));
#ifndef QT_NO_TOOLTIP
    _up->setToolTip(QApplication::translate("AScope", "Scroll the display up.", 0));
#endif // QT_NO_TOOLTIP
    _up->setText(QApplication::translate("AScope", "^", 0));
#ifndef QT_NO_TOOLTIP
    _dn->setToolTip(QApplication::translate("AScope", "Scroll the display down.", 0));
#endif // QT_NO_TOOLTIP
    _dn->setText(QApplication::translate("AScope", "v", 0));
#ifndef QT_NO_TOOLTIP
    _gainKnob->setToolTip(QApplication::translate("AScope", "Change the vertical range of the display.", 0));
#endif // QT_NO_TOOLTIP
  } // retranslateUi

};

namespace Ui {
  class AScope: public Ui_AScope {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_ASCOPE_H

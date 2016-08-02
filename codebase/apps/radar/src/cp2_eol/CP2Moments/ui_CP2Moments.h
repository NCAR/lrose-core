/********************************************************************************
** Form generated from reading ui file 'CP2Moments.ui'
**
** Created: Wed Aug 1 11:47:16 2007
**      by: Qt User Interface Compiler version 4.2.2
**
** WARNING! All changes made in this file will be lost when recompiling ui file!
********************************************************************************/

#ifndef UI_CP2MOMENTS_H
#define UI_CP2MOMENTS_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QCheckBox>
#include <QtGui/QDialog>
#include <QtGui/QDialogButtonBox>
#include <QtGui/QGridLayout>
#include <QtGui/QGroupBox>
#include <QtGui/QHBoxLayout>
#include <QtGui/QLabel>
#include <QtGui/QSpacerItem>
#include <QtGui/QVBoxLayout>

class Ui_CP2Moments
{
public:
    QVBoxLayout *vboxLayout;
    QGroupBox *groupBox;
    QVBoxLayout *vboxLayout1;
    QLabel *_statusText;
    QGroupBox *groupBox_6;
    QHBoxLayout *hboxLayout;
    QVBoxLayout *vboxLayout2;
    QLabel *label_3;
    QGridLayout *gridLayout;
    QLabel *label_2;
    QLabel *_sAzIndexed;
    QCheckBox *_sBiQuadCheck;
    QLabel *label;
    QLabel *label_7;
    QLabel *_sClutterFilter;
    QLabel *label_9;
    QLabel *_sPulsesPerBeam;
    QSpacerItem *spacerItem;
    QVBoxLayout *vboxLayout3;
    QLabel *label_4;
    QGridLayout *gridLayout1;
    QLabel *label_5;
    QLabel *label_8;
    QCheckBox *_xBiQuadCheck;
    QLabel *label_10;
    QLabel *_xPulsesPerBeam;
    QLabel *_xClutterFilter;
    QLabel *label_6;
    QLabel *_xAzIndexed;
    QGroupBox *groupBox_2;
    QHBoxLayout *hboxLayout1;
    QHBoxLayout *hboxLayout2;
    QHBoxLayout *hboxLayout3;
    QLabel *textLabel4;
    QLabel *_sBeams;
    QSpacerItem *spacerItem1;
    QHBoxLayout *hboxLayout4;
    QLabel *textLabel5;
    QLabel *_xBeams;
    QGroupBox *groupBox_3;
    QVBoxLayout *vboxLayout4;
    QGridLayout *gridLayout2;
    QLabel *_piraq0rate;
    QLabel *textLabel1_3;
    QLabel *_piraq1rate;
    QLabel *_piraq2rate;
    QLabel *textLabel1;
    QLabel *textLabel3;
    QLabel *textLabel1_2;
    QGroupBox *groupBox_4;
    QVBoxLayout *vboxLayout5;
    QGridLayout *gridLayout3;
    QLabel *textLabel1_2_2;
    QLabel *_piraq2Errors;
    QLabel *_piraq0Errors;
    QLabel *textLabel1_4;
    QLabel *textLabel1_3_2;
    QLabel *textLabel2;
    QLabel *_piraq1Errors;
    QHBoxLayout *hboxLayout5;
    QLabel *textLabel6;
    QLabel *_collatorErrors;
    QGroupBox *groupBox_5;
    QVBoxLayout *vboxLayout6;
    QHBoxLayout *hboxLayout6;
    QHBoxLayout *hboxLayout7;
    QLabel *_inIpText;
    QLabel *_inPortText;
    QSpacerItem *spacerItem2;
    QHBoxLayout *hboxLayout8;
    QLabel *_outIpText;
    QLabel *_outPortText;
    QDialogButtonBox *buttonBox;

    void setupUi(QDialog *CP2Moments)
    {
    CP2Moments->setObjectName(QString::fromUtf8("CP2Moments"));
    vboxLayout = new QVBoxLayout(CP2Moments);
    vboxLayout->setSpacing(6);
    vboxLayout->setMargin(9);
    vboxLayout->setObjectName(QString::fromUtf8("vboxLayout"));
    groupBox = new QGroupBox(CP2Moments);
    groupBox->setObjectName(QString::fromUtf8("groupBox"));
    vboxLayout1 = new QVBoxLayout(groupBox);
    vboxLayout1->setSpacing(6);
    vboxLayout1->setMargin(9);
    vboxLayout1->setObjectName(QString::fromUtf8("vboxLayout1"));
    _statusText = new QLabel(groupBox);
    _statusText->setObjectName(QString::fromUtf8("_statusText"));
    _statusText->setAlignment(Qt::AlignCenter);
    _statusText->setWordWrap(false);

    vboxLayout1->addWidget(_statusText);


    vboxLayout->addWidget(groupBox);

    groupBox_6 = new QGroupBox(CP2Moments);
    groupBox_6->setObjectName(QString::fromUtf8("groupBox_6"));
    hboxLayout = new QHBoxLayout(groupBox_6);
    hboxLayout->setSpacing(6);
    hboxLayout->setMargin(9);
    hboxLayout->setObjectName(QString::fromUtf8("hboxLayout"));
    vboxLayout2 = new QVBoxLayout();
    vboxLayout2->setSpacing(6);
    vboxLayout2->setMargin(0);
    vboxLayout2->setObjectName(QString::fromUtf8("vboxLayout2"));
    label_3 = new QLabel(groupBox_6);
    label_3->setObjectName(QString::fromUtf8("label_3"));
    label_3->setTextFormat(Qt::RichText);
    label_3->setAlignment(Qt::AlignCenter);

    vboxLayout2->addWidget(label_3);

    gridLayout = new QGridLayout();
    gridLayout->setSpacing(6);
    gridLayout->setMargin(0);
    gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
    label_2 = new QLabel(groupBox_6);
    label_2->setObjectName(QString::fromUtf8("label_2"));

    gridLayout->addWidget(label_2, 3, 0, 1, 2);

    _sAzIndexed = new QLabel(groupBox_6);
    _sAzIndexed->setObjectName(QString::fromUtf8("_sAzIndexed"));

    gridLayout->addWidget(_sAzIndexed, 1, 2, 1, 1);

    _sBiQuadCheck = new QCheckBox(groupBox_6);
    _sBiQuadCheck->setObjectName(QString::fromUtf8("_sBiQuadCheck"));
    _sBiQuadCheck->setLayoutDirection(Qt::RightToLeft);

    gridLayout->addWidget(_sBiQuadCheck, 2, 1, 1, 2);

    label = new QLabel(groupBox_6);
    label->setObjectName(QString::fromUtf8("label"));

    gridLayout->addWidget(label, 0, 0, 1, 2);

    label_7 = new QLabel(groupBox_6);
    label_7->setObjectName(QString::fromUtf8("label_7"));

    gridLayout->addWidget(label_7, 1, 0, 1, 2);

    _sClutterFilter = new QLabel(groupBox_6);
    _sClutterFilter->setObjectName(QString::fromUtf8("_sClutterFilter"));

    gridLayout->addWidget(_sClutterFilter, 3, 2, 1, 1);

    label_9 = new QLabel(groupBox_6);
    label_9->setObjectName(QString::fromUtf8("label_9"));

    gridLayout->addWidget(label_9, 2, 0, 1, 1);

    _sPulsesPerBeam = new QLabel(groupBox_6);
    _sPulsesPerBeam->setObjectName(QString::fromUtf8("_sPulsesPerBeam"));

    gridLayout->addWidget(_sPulsesPerBeam, 0, 2, 1, 1);


    vboxLayout2->addLayout(gridLayout);


    hboxLayout->addLayout(vboxLayout2);

    spacerItem = new QSpacerItem(31, 80, QSizePolicy::Expanding, QSizePolicy::Minimum);

    hboxLayout->addItem(spacerItem);

    vboxLayout3 = new QVBoxLayout();
    vboxLayout3->setSpacing(6);
    vboxLayout3->setMargin(0);
    vboxLayout3->setObjectName(QString::fromUtf8("vboxLayout3"));
    label_4 = new QLabel(groupBox_6);
    label_4->setObjectName(QString::fromUtf8("label_4"));
    label_4->setTextFormat(Qt::RichText);
    label_4->setAlignment(Qt::AlignCenter);

    vboxLayout3->addWidget(label_4);

    gridLayout1 = new QGridLayout();
    gridLayout1->setSpacing(6);
    gridLayout1->setMargin(0);
    gridLayout1->setObjectName(QString::fromUtf8("gridLayout1"));
    label_5 = new QLabel(groupBox_6);
    label_5->setObjectName(QString::fromUtf8("label_5"));

    gridLayout1->addWidget(label_5, 3, 0, 1, 1);

    label_8 = new QLabel(groupBox_6);
    label_8->setObjectName(QString::fromUtf8("label_8"));

    gridLayout1->addWidget(label_8, 1, 0, 1, 1);

    _xBiQuadCheck = new QCheckBox(groupBox_6);
    _xBiQuadCheck->setObjectName(QString::fromUtf8("_xBiQuadCheck"));
    _xBiQuadCheck->setLayoutDirection(Qt::RightToLeft);

    gridLayout1->addWidget(_xBiQuadCheck, 2, 1, 1, 1);

    label_10 = new QLabel(groupBox_6);
    label_10->setObjectName(QString::fromUtf8("label_10"));

    gridLayout1->addWidget(label_10, 2, 0, 1, 1);

    _xPulsesPerBeam = new QLabel(groupBox_6);
    _xPulsesPerBeam->setObjectName(QString::fromUtf8("_xPulsesPerBeam"));

    gridLayout1->addWidget(_xPulsesPerBeam, 0, 1, 1, 1);

    _xClutterFilter = new QLabel(groupBox_6);
    _xClutterFilter->setObjectName(QString::fromUtf8("_xClutterFilter"));

    gridLayout1->addWidget(_xClutterFilter, 3, 1, 1, 1);

    label_6 = new QLabel(groupBox_6);
    label_6->setObjectName(QString::fromUtf8("label_6"));

    gridLayout1->addWidget(label_6, 0, 0, 1, 1);

    _xAzIndexed = new QLabel(groupBox_6);
    _xAzIndexed->setObjectName(QString::fromUtf8("_xAzIndexed"));

    gridLayout1->addWidget(_xAzIndexed, 1, 1, 1, 1);


    vboxLayout3->addLayout(gridLayout1);


    hboxLayout->addLayout(vboxLayout3);


    vboxLayout->addWidget(groupBox_6);

    groupBox_2 = new QGroupBox(CP2Moments);
    groupBox_2->setObjectName(QString::fromUtf8("groupBox_2"));
    hboxLayout1 = new QHBoxLayout(groupBox_2);
    hboxLayout1->setSpacing(6);
    hboxLayout1->setMargin(9);
    hboxLayout1->setObjectName(QString::fromUtf8("hboxLayout1"));
    hboxLayout2 = new QHBoxLayout();
    hboxLayout2->setSpacing(6);
    hboxLayout2->setMargin(0);
    hboxLayout2->setObjectName(QString::fromUtf8("hboxLayout2"));
    hboxLayout3 = new QHBoxLayout();
    hboxLayout3->setSpacing(6);
    hboxLayout3->setMargin(0);
    hboxLayout3->setObjectName(QString::fromUtf8("hboxLayout3"));
    textLabel4 = new QLabel(groupBox_2);
    textLabel4->setObjectName(QString::fromUtf8("textLabel4"));
    textLabel4->setWordWrap(false);

    hboxLayout3->addWidget(textLabel4);

    _sBeams = new QLabel(groupBox_2);
    _sBeams->setObjectName(QString::fromUtf8("_sBeams"));
    _sBeams->setWordWrap(false);

    hboxLayout3->addWidget(_sBeams);


    hboxLayout2->addLayout(hboxLayout3);

    spacerItem1 = new QSpacerItem(20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

    hboxLayout2->addItem(spacerItem1);

    hboxLayout4 = new QHBoxLayout();
    hboxLayout4->setSpacing(6);
    hboxLayout4->setMargin(0);
    hboxLayout4->setObjectName(QString::fromUtf8("hboxLayout4"));
    textLabel5 = new QLabel(groupBox_2);
    textLabel5->setObjectName(QString::fromUtf8("textLabel5"));
    textLabel5->setWordWrap(false);

    hboxLayout4->addWidget(textLabel5);

    _xBeams = new QLabel(groupBox_2);
    _xBeams->setObjectName(QString::fromUtf8("_xBeams"));
    _xBeams->setWordWrap(false);

    hboxLayout4->addWidget(_xBeams);


    hboxLayout2->addLayout(hboxLayout4);


    hboxLayout1->addLayout(hboxLayout2);


    vboxLayout->addWidget(groupBox_2);

    groupBox_3 = new QGroupBox(CP2Moments);
    groupBox_3->setObjectName(QString::fromUtf8("groupBox_3"));
    vboxLayout4 = new QVBoxLayout(groupBox_3);
    vboxLayout4->setSpacing(6);
    vboxLayout4->setMargin(9);
    vboxLayout4->setObjectName(QString::fromUtf8("vboxLayout4"));
    gridLayout2 = new QGridLayout();
    gridLayout2->setSpacing(6);
    gridLayout2->setMargin(0);
    gridLayout2->setObjectName(QString::fromUtf8("gridLayout2"));
    _piraq0rate = new QLabel(groupBox_3);
    _piraq0rate->setObjectName(QString::fromUtf8("_piraq0rate"));
    _piraq0rate->setAlignment(Qt::AlignCenter);
    _piraq0rate->setWordWrap(false);

    gridLayout2->addWidget(_piraq0rate, 1, 1, 1, 1);

    textLabel1_3 = new QLabel(groupBox_3);
    textLabel1_3->setObjectName(QString::fromUtf8("textLabel1_3"));
    textLabel1_3->setAlignment(Qt::AlignCenter);
    textLabel1_3->setWordWrap(true);

    gridLayout2->addWidget(textLabel1_3, 0, 2, 1, 1);

    _piraq1rate = new QLabel(groupBox_3);
    _piraq1rate->setObjectName(QString::fromUtf8("_piraq1rate"));
    _piraq1rate->setAlignment(Qt::AlignCenter);
    _piraq1rate->setWordWrap(false);

    gridLayout2->addWidget(_piraq1rate, 1, 2, 1, 1);

    _piraq2rate = new QLabel(groupBox_3);
    _piraq2rate->setObjectName(QString::fromUtf8("_piraq2rate"));
    _piraq2rate->setAlignment(Qt::AlignCenter);
    _piraq2rate->setWordWrap(false);

    gridLayout2->addWidget(_piraq2rate, 1, 3, 1, 1);

    textLabel1 = new QLabel(groupBox_3);
    textLabel1->setObjectName(QString::fromUtf8("textLabel1"));
    textLabel1->setAlignment(Qt::AlignCenter);
    textLabel1->setWordWrap(true);

    gridLayout2->addWidget(textLabel1, 0, 1, 1, 1);

    textLabel3 = new QLabel(groupBox_3);
    textLabel3->setObjectName(QString::fromUtf8("textLabel3"));
    textLabel3->setAlignment(Qt::AlignCenter);
    textLabel3->setWordWrap(true);

    gridLayout2->addWidget(textLabel3, 1, 0, 1, 1);

    textLabel1_2 = new QLabel(groupBox_3);
    textLabel1_2->setObjectName(QString::fromUtf8("textLabel1_2"));
    textLabel1_2->setAlignment(Qt::AlignCenter);
    textLabel1_2->setWordWrap(true);

    gridLayout2->addWidget(textLabel1_2, 0, 3, 1, 1);


    vboxLayout4->addLayout(gridLayout2);


    vboxLayout->addWidget(groupBox_3);

    groupBox_4 = new QGroupBox(CP2Moments);
    groupBox_4->setObjectName(QString::fromUtf8("groupBox_4"));
    vboxLayout5 = new QVBoxLayout(groupBox_4);
    vboxLayout5->setSpacing(6);
    vboxLayout5->setMargin(9);
    vboxLayout5->setObjectName(QString::fromUtf8("vboxLayout5"));
    gridLayout3 = new QGridLayout();
    gridLayout3->setSpacing(6);
    gridLayout3->setMargin(0);
    gridLayout3->setObjectName(QString::fromUtf8("gridLayout3"));
    textLabel1_2_2 = new QLabel(groupBox_4);
    textLabel1_2_2->setObjectName(QString::fromUtf8("textLabel1_2_2"));
    textLabel1_2_2->setAlignment(Qt::AlignCenter);
    textLabel1_2_2->setWordWrap(true);

    gridLayout3->addWidget(textLabel1_2_2, 0, 3, 1, 1);

    _piraq2Errors = new QLabel(groupBox_4);
    _piraq2Errors->setObjectName(QString::fromUtf8("_piraq2Errors"));
    _piraq2Errors->setAlignment(Qt::AlignCenter);
    _piraq2Errors->setWordWrap(false);

    gridLayout3->addWidget(_piraq2Errors, 1, 3, 1, 1);

    _piraq0Errors = new QLabel(groupBox_4);
    _piraq0Errors->setObjectName(QString::fromUtf8("_piraq0Errors"));
    _piraq0Errors->setAlignment(Qt::AlignCenter);
    _piraq0Errors->setWordWrap(false);

    gridLayout3->addWidget(_piraq0Errors, 1, 1, 1, 1);

    textLabel1_4 = new QLabel(groupBox_4);
    textLabel1_4->setObjectName(QString::fromUtf8("textLabel1_4"));
    textLabel1_4->setAlignment(Qt::AlignCenter);
    textLabel1_4->setWordWrap(true);

    gridLayout3->addWidget(textLabel1_4, 0, 1, 1, 1);

    textLabel1_3_2 = new QLabel(groupBox_4);
    textLabel1_3_2->setObjectName(QString::fromUtf8("textLabel1_3_2"));
    textLabel1_3_2->setAlignment(Qt::AlignCenter);
    textLabel1_3_2->setWordWrap(true);

    gridLayout3->addWidget(textLabel1_3_2, 0, 2, 1, 1);

    textLabel2 = new QLabel(groupBox_4);
    textLabel2->setObjectName(QString::fromUtf8("textLabel2"));
    textLabel2->setAlignment(Qt::AlignCenter);
    textLabel2->setWordWrap(true);

    gridLayout3->addWidget(textLabel2, 1, 0, 1, 1);

    _piraq1Errors = new QLabel(groupBox_4);
    _piraq1Errors->setObjectName(QString::fromUtf8("_piraq1Errors"));
    _piraq1Errors->setAlignment(Qt::AlignCenter);
    _piraq1Errors->setWordWrap(false);

    gridLayout3->addWidget(_piraq1Errors, 1, 2, 1, 1);


    vboxLayout5->addLayout(gridLayout3);

    hboxLayout5 = new QHBoxLayout();
    hboxLayout5->setSpacing(6);
    hboxLayout5->setMargin(0);
    hboxLayout5->setObjectName(QString::fromUtf8("hboxLayout5"));
    textLabel6 = new QLabel(groupBox_4);
    textLabel6->setObjectName(QString::fromUtf8("textLabel6"));
    textLabel6->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
    textLabel6->setWordWrap(true);

    hboxLayout5->addWidget(textLabel6);

    _collatorErrors = new QLabel(groupBox_4);
    _collatorErrors->setObjectName(QString::fromUtf8("_collatorErrors"));
    _collatorErrors->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignVCenter);
    _collatorErrors->setWordWrap(false);

    hboxLayout5->addWidget(_collatorErrors);


    vboxLayout5->addLayout(hboxLayout5);


    vboxLayout->addWidget(groupBox_4);

    groupBox_5 = new QGroupBox(CP2Moments);
    groupBox_5->setObjectName(QString::fromUtf8("groupBox_5"));
    vboxLayout6 = new QVBoxLayout(groupBox_5);
    vboxLayout6->setSpacing(6);
    vboxLayout6->setMargin(9);
    vboxLayout6->setObjectName(QString::fromUtf8("vboxLayout6"));
    hboxLayout6 = new QHBoxLayout();
    hboxLayout6->setSpacing(6);
    hboxLayout6->setMargin(0);
    hboxLayout6->setObjectName(QString::fromUtf8("hboxLayout6"));
    hboxLayout7 = new QHBoxLayout();
    hboxLayout7->setSpacing(6);
    hboxLayout7->setMargin(0);
    hboxLayout7->setObjectName(QString::fromUtf8("hboxLayout7"));
    _inIpText = new QLabel(groupBox_5);
    _inIpText->setObjectName(QString::fromUtf8("_inIpText"));

    hboxLayout7->addWidget(_inIpText);

    _inPortText = new QLabel(groupBox_5);
    _inPortText->setObjectName(QString::fromUtf8("_inPortText"));

    hboxLayout7->addWidget(_inPortText);


    hboxLayout6->addLayout(hboxLayout7);

    spacerItem2 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

    hboxLayout6->addItem(spacerItem2);

    hboxLayout8 = new QHBoxLayout();
    hboxLayout8->setSpacing(6);
    hboxLayout8->setMargin(0);
    hboxLayout8->setObjectName(QString::fromUtf8("hboxLayout8"));
    _outIpText = new QLabel(groupBox_5);
    _outIpText->setObjectName(QString::fromUtf8("_outIpText"));

    hboxLayout8->addWidget(_outIpText);

    _outPortText = new QLabel(groupBox_5);
    _outPortText->setObjectName(QString::fromUtf8("_outPortText"));

    hboxLayout8->addWidget(_outPortText);


    hboxLayout6->addLayout(hboxLayout8);


    vboxLayout6->addLayout(hboxLayout6);


    vboxLayout->addWidget(groupBox_5);

    buttonBox = new QDialogButtonBox(CP2Moments);
    buttonBox->setObjectName(QString::fromUtf8("buttonBox"));
    buttonBox->setOrientation(Qt::Horizontal);
    buttonBox->setStandardButtons(QDialogButtonBox::Close);
    buttonBox->setCenterButtons(true);

    vboxLayout->addWidget(buttonBox);


    retranslateUi(CP2Moments);

    QSize size(432, 558);
    size = size.expandedTo(CP2Moments->minimumSizeHint());
    CP2Moments->resize(size);

    QObject::connect(buttonBox, SIGNAL(accepted()), CP2Moments, SLOT(accept()));
    QObject::connect(buttonBox, SIGNAL(rejected()), CP2Moments, SLOT(reject()));

    QMetaObject::connectSlotsByName(CP2Moments);
    } // setupUi

    void retranslateUi(QDialog *CP2Moments)
    {
    CP2Moments->setWindowTitle(QApplication::translate("CP2Moments", "CP2Moments", 0, QApplication::UnicodeUTF8));
    groupBox->setTitle(QApplication::translate("CP2Moments", "Status", 0, QApplication::UnicodeUTF8));
    _statusText->setText(QApplication::translate("CP2Moments", "s", 0, QApplication::UnicodeUTF8));
    groupBox_6->setTitle(QApplication::translate("CP2Moments", "Parameters", 0, QApplication::UnicodeUTF8));
    label_3->setText(QApplication::translate("CP2Moments", "<font size=\"+1\"><b>S Band</b></font>", 0, QApplication::UnicodeUTF8));
    label_2->setText(QApplication::translate("CP2Moments", "FFT Clutter Filter", 0, QApplication::UnicodeUTF8));
    _sAzIndexed->setText(QApplication::translate("CP2Moments", "Off", 0, QApplication::UnicodeUTF8));
    _sBiQuadCheck->setText(QString());
    label->setText(QApplication::translate("CP2Moments", "Pulses per Beam", 0, QApplication::UnicodeUTF8));
    label_7->setText(QApplication::translate("CP2Moments", "Az Indexed Beams", 0, QApplication::UnicodeUTF8));
    _sClutterFilter->setText(QApplication::translate("CP2Moments", "Off", 0, QApplication::UnicodeUTF8));
    label_9->setText(QApplication::translate("CP2Moments", "BiQuad IIR Filter", 0, QApplication::UnicodeUTF8));
    _sPulsesPerBeam->setText(QApplication::translate("CP2Moments", "0", 0, QApplication::UnicodeUTF8));
    label_4->setText(QApplication::translate("CP2Moments", "<font size=\"+1\"><b>X Band</b></font>", 0, QApplication::UnicodeUTF8));
    label_5->setText(QApplication::translate("CP2Moments", "FFT Clutter Filter", 0, QApplication::UnicodeUTF8));
    label_8->setText(QApplication::translate("CP2Moments", "Az Indexed Beams", 0, QApplication::UnicodeUTF8));
    _xBiQuadCheck->setText(QString());
    label_10->setText(QApplication::translate("CP2Moments", "BiQuad IIR Filter", 0, QApplication::UnicodeUTF8));
    _xPulsesPerBeam->setText(QApplication::translate("CP2Moments", "0", 0, QApplication::UnicodeUTF8));
    _xClutterFilter->setText(QApplication::translate("CP2Moments", "Off", 0, QApplication::UnicodeUTF8));
    label_6->setText(QApplication::translate("CP2Moments", "Pulses per Beam", 0, QApplication::UnicodeUTF8));
    _xAzIndexed->setText(QApplication::translate("CP2Moments", "Off", 0, QApplication::UnicodeUTF8));
    groupBox_2->setTitle(QApplication::translate("CP2Moments", "Beam Counts", 0, QApplication::UnicodeUTF8));
    textLabel4->setText(QApplication::translate("CP2Moments", "<font size=\"+1\"><b>S Band</b></font>", 0, QApplication::UnicodeUTF8));
    _sBeams->setText(QApplication::translate("CP2Moments", "b", 0, QApplication::UnicodeUTF8));
    textLabel5->setText(QApplication::translate("CP2Moments", "<font size=\"+1\"><b>X Band</b></font>", 0, QApplication::UnicodeUTF8));
    _xBeams->setText(QApplication::translate("CP2Moments", "b", 0, QApplication::UnicodeUTF8));
    groupBox_3->setTitle(QApplication::translate("CP2Moments", "Throughput", 0, QApplication::UnicodeUTF8));
    _piraq0rate->setText(QApplication::translate("CP2Moments", "r", 0, QApplication::UnicodeUTF8));
    textLabel1_3->setText(QApplication::translate("CP2Moments", "<font size=\"+1\"><b>Xh</b></font>", 0, QApplication::UnicodeUTF8));
    _piraq1rate->setText(QApplication::translate("CP2Moments", "r", 0, QApplication::UnicodeUTF8));
    _piraq2rate->setText(QApplication::translate("CP2Moments", "r", 0, QApplication::UnicodeUTF8));
    textLabel1->setText(QApplication::translate("CP2Moments", "<font size=\"+1\"><b>S</b></font>", 0, QApplication::UnicodeUTF8));
    textLabel3->setText(QApplication::translate("CP2Moments", "<font size=\"+1\"><b>Pulse Rate\n"
"(/s)</b></font>", 0, QApplication::UnicodeUTF8));
    textLabel1_2->setText(QApplication::translate("CP2Moments", "<font size=\"+1\"><b>Xv</b></font>", 0, QApplication::UnicodeUTF8));
    groupBox_4->setTitle(QApplication::translate("CP2Moments", "Errors", 0, QApplication::UnicodeUTF8));
    textLabel1_2_2->setText(QApplication::translate("CP2Moments", "<font size=\"+1\"><b>Xv</b></font>", 0, QApplication::UnicodeUTF8));
    _piraq2Errors->setText(QApplication::translate("CP2Moments", "n", 0, QApplication::UnicodeUTF8));
    _piraq0Errors->setText(QApplication::translate("CP2Moments", "n", 0, QApplication::UnicodeUTF8));
    textLabel1_4->setText(QApplication::translate("CP2Moments", "<font size=\"+1\"><b>S</b></font>", 0, QApplication::UnicodeUTF8));
    textLabel1_3_2->setText(QApplication::translate("CP2Moments", "<font size=\"+1\"><b>Xh</b></font>", 0, QApplication::UnicodeUTF8));
    textLabel2->setText(QApplication::translate("CP2Moments", "<font size=\"+1\"><b>Pulse Number \n"
"Errors</b></font>", 0, QApplication::UnicodeUTF8));
    _piraq1Errors->setText(QApplication::translate("CP2Moments", "n", 0, QApplication::UnicodeUTF8));
    textLabel6->setText(QApplication::translate("CP2Moments", "<font size=\"+1\"><b>Collator</b></font>", 0, QApplication::UnicodeUTF8));
    _collatorErrors->setText(QApplication::translate("CP2Moments", "n", 0, QApplication::UnicodeUTF8));
    groupBox_5->setTitle(QApplication::translate("CP2Moments", "Pulse and Product Network Interfaces", 0, QApplication::UnicodeUTF8));
    _inIpText->setText(QApplication::translate("CP2Moments", "Network unavailable", 0, QApplication::UnicodeUTF8));
    _inPortText->setText(QApplication::translate("CP2Moments", "Port unavailable", 0, QApplication::UnicodeUTF8));
    _outIpText->setText(QApplication::translate("CP2Moments", "Network unavailable", 0, QApplication::UnicodeUTF8));
    _outPortText->setText(QApplication::translate("CP2Moments", "Port unavailable", 0, QApplication::UnicodeUTF8));
    Q_UNUSED(CP2Moments);
    } // retranslateUi

};

namespace Ui {
    class CP2Moments: public Ui_CP2Moments {};
} // namespace Ui

#endif // UI_CP2MOMENTS_H

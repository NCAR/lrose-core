/********************************************************************************
** Form generated from reading ui file 'CP2PPI.ui'
**
** Created: Wed Aug 1 11:46:59 2007
**      by: Qt User Interface Compiler version 4.2.2
**
** WARNING! All changes made in this file will be lost when recompiling ui file!
********************************************************************************/

#ifndef UI_CP2PPI_H
#define UI_CP2PPI_H

#include "ColorBar.h"
#include "PPI.h"
#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QCheckBox>
#include <QtGui/QHBoxLayout>
#include <QtGui/QLCDNumber>
#include <QtGui/QLabel>
#include <QtGui/QPushButton>
#include <QtGui/QSpacerItem>
#include <QtGui/QStackedWidget>
#include <QtGui/QTabWidget>
#include <QtGui/QVBoxLayout>
#include <QtGui/QWidget>

class Ui_CP2PPI
{
public:
    QHBoxLayout *hboxLayout;
    QHBoxLayout *hboxLayout1;
    QStackedWidget *_ppiStack;
    QWidget *page;
    QVBoxLayout *vboxLayout;
    PPI *_ppiS;
    QWidget *page_2;
    QVBoxLayout *vboxLayout1;
    PPI *_ppiX;
    ColorBar *_colorBar;
    QVBoxLayout *vboxLayout2;
    QVBoxLayout *vboxLayout3;
    QPushButton *_pauseButton;
    QPushButton *_zoomInButton;
    QPushButton *_resetButton;
    QPushButton *_zoomOutButton;
    QHBoxLayout *hboxLayout2;
    QLabel *textLabel1;
    QLCDNumber *ZoomFactor;
    QHBoxLayout *hboxLayout3;
    QCheckBox *_ringsCheckBox;
    QCheckBox *_gridsCheckBox;
    QVBoxLayout *vboxLayout4;
    QPushButton *_colorButton;
    QPushButton *_ringColorButton;
    QPushButton *_saveButton;
    QTabWidget *_typeTab;
    QWidget *tab;
    QSpacerItem *spacerItem;
    QHBoxLayout *hboxLayout4;
    QLabel *textLabel1_3;
    QLCDNumber *_azLCD;
    QHBoxLayout *hboxLayout5;
    QLabel *textLabel1_4;
    QLCDNumber *_elLCD;
    QHBoxLayout *hboxLayout6;
    QLabel *_textIP;
    QLabel *_textPort;

    void setupUi(QWidget *CP2PPI)
    {
    CP2PPI->setObjectName(QString::fromUtf8("CP2PPI"));
    QSizePolicy sizePolicy(static_cast<QSizePolicy::Policy>(1), static_cast<QSizePolicy::Policy>(1));
    sizePolicy.setHorizontalStretch(0);
    sizePolicy.setVerticalStretch(0);
    sizePolicy.setHeightForWidth(CP2PPI->sizePolicy().hasHeightForWidth());
    CP2PPI->setSizePolicy(sizePolicy);
    hboxLayout = new QHBoxLayout(CP2PPI);
    hboxLayout->setSpacing(6);
    hboxLayout->setMargin(9);
    hboxLayout->setObjectName(QString::fromUtf8("hboxLayout"));
    hboxLayout1 = new QHBoxLayout();
    hboxLayout1->setSpacing(6);
    hboxLayout1->setMargin(0);
    hboxLayout1->setObjectName(QString::fromUtf8("hboxLayout1"));
    _ppiStack = new QStackedWidget(CP2PPI);
    _ppiStack->setObjectName(QString::fromUtf8("_ppiStack"));
    QSizePolicy sizePolicy1(static_cast<QSizePolicy::Policy>(3), static_cast<QSizePolicy::Policy>(3));
    sizePolicy1.setHorizontalStretch(0);
    sizePolicy1.setVerticalStretch(0);
    sizePolicy1.setHeightForWidth(_ppiStack->sizePolicy().hasHeightForWidth());
    _ppiStack->setSizePolicy(sizePolicy1);
    _ppiStack->setMinimumSize(QSize(600, 600));
    page = new QWidget();
    page->setObjectName(QString::fromUtf8("page"));
    vboxLayout = new QVBoxLayout(page);
    vboxLayout->setSpacing(6);
    vboxLayout->setMargin(9);
    vboxLayout->setObjectName(QString::fromUtf8("vboxLayout"));
    _ppiS = new PPI(page);
    _ppiS->setObjectName(QString::fromUtf8("_ppiS"));
    QSizePolicy sizePolicy2(static_cast<QSizePolicy::Policy>(3), static_cast<QSizePolicy::Policy>(3));
    sizePolicy2.setHorizontalStretch(0);
    sizePolicy2.setVerticalStretch(0);
    sizePolicy2.setHeightForWidth(_ppiS->sizePolicy().hasHeightForWidth());
    _ppiS->setSizePolicy(sizePolicy2);
    _ppiS->setMinimumSize(QSize(600, 600));

    vboxLayout->addWidget(_ppiS);

    _ppiStack->addWidget(page);
    page_2 = new QWidget();
    page_2->setObjectName(QString::fromUtf8("page_2"));
    vboxLayout1 = new QVBoxLayout(page_2);
    vboxLayout1->setSpacing(6);
    vboxLayout1->setMargin(9);
    vboxLayout1->setObjectName(QString::fromUtf8("vboxLayout1"));
    _ppiX = new PPI(page_2);
    _ppiX->setObjectName(QString::fromUtf8("_ppiX"));
    QSizePolicy sizePolicy3(static_cast<QSizePolicy::Policy>(3), static_cast<QSizePolicy::Policy>(3));
    sizePolicy3.setHorizontalStretch(0);
    sizePolicy3.setVerticalStretch(0);
    sizePolicy3.setHeightForWidth(_ppiX->sizePolicy().hasHeightForWidth());
    _ppiX->setSizePolicy(sizePolicy3);
    _ppiX->setMinimumSize(QSize(600, 600));

    vboxLayout1->addWidget(_ppiX);

    _ppiStack->addWidget(page_2);

    hboxLayout1->addWidget(_ppiStack);

    _colorBar = new ColorBar(CP2PPI);
    _colorBar->setObjectName(QString::fromUtf8("_colorBar"));
    QSizePolicy sizePolicy4(static_cast<QSizePolicy::Policy>(0), static_cast<QSizePolicy::Policy>(3));
    sizePolicy4.setHorizontalStretch(0);
    sizePolicy4.setVerticalStretch(0);
    sizePolicy4.setHeightForWidth(_colorBar->sizePolicy().hasHeightForWidth());
    _colorBar->setSizePolicy(sizePolicy4);
    _colorBar->setMinimumSize(QSize(60, 100));

    hboxLayout1->addWidget(_colorBar);


    hboxLayout->addLayout(hboxLayout1);

    vboxLayout2 = new QVBoxLayout();
    vboxLayout2->setSpacing(6);
    vboxLayout2->setMargin(0);
    vboxLayout2->setObjectName(QString::fromUtf8("vboxLayout2"));
    vboxLayout3 = new QVBoxLayout();
    vboxLayout3->setSpacing(6);
    vboxLayout3->setMargin(0);
    vboxLayout3->setObjectName(QString::fromUtf8("vboxLayout3"));
    _pauseButton = new QPushButton(CP2PPI);
    _pauseButton->setObjectName(QString::fromUtf8("_pauseButton"));
    QSizePolicy sizePolicy5(static_cast<QSizePolicy::Policy>(1), static_cast<QSizePolicy::Policy>(1));
    sizePolicy5.setHorizontalStretch(0);
    sizePolicy5.setVerticalStretch(0);
    sizePolicy5.setHeightForWidth(_pauseButton->sizePolicy().hasHeightForWidth());
    _pauseButton->setSizePolicy(sizePolicy5);
    _pauseButton->setMinimumSize(QSize(100, 0));
    _pauseButton->setCheckable(true);

    vboxLayout3->addWidget(_pauseButton);

    _zoomInButton = new QPushButton(CP2PPI);
    _zoomInButton->setObjectName(QString::fromUtf8("_zoomInButton"));
    QSizePolicy sizePolicy6(static_cast<QSizePolicy::Policy>(1), static_cast<QSizePolicy::Policy>(1));
    sizePolicy6.setHorizontalStretch(0);
    sizePolicy6.setVerticalStretch(0);
    sizePolicy6.setHeightForWidth(_zoomInButton->sizePolicy().hasHeightForWidth());
    _zoomInButton->setSizePolicy(sizePolicy6);
    _zoomInButton->setMinimumSize(QSize(100, 0));

    vboxLayout3->addWidget(_zoomInButton);

    _resetButton = new QPushButton(CP2PPI);
    _resetButton->setObjectName(QString::fromUtf8("_resetButton"));
    QSizePolicy sizePolicy7(static_cast<QSizePolicy::Policy>(1), static_cast<QSizePolicy::Policy>(1));
    sizePolicy7.setHorizontalStretch(0);
    sizePolicy7.setVerticalStretch(0);
    sizePolicy7.setHeightForWidth(_resetButton->sizePolicy().hasHeightForWidth());
    _resetButton->setSizePolicy(sizePolicy7);
    _resetButton->setMinimumSize(QSize(100, 0));

    vboxLayout3->addWidget(_resetButton);

    _zoomOutButton = new QPushButton(CP2PPI);
    _zoomOutButton->setObjectName(QString::fromUtf8("_zoomOutButton"));
    QSizePolicy sizePolicy8(static_cast<QSizePolicy::Policy>(1), static_cast<QSizePolicy::Policy>(1));
    sizePolicy8.setHorizontalStretch(0);
    sizePolicy8.setVerticalStretch(0);
    sizePolicy8.setHeightForWidth(_zoomOutButton->sizePolicy().hasHeightForWidth());
    _zoomOutButton->setSizePolicy(sizePolicy8);
    _zoomOutButton->setMinimumSize(QSize(100, 0));

    vboxLayout3->addWidget(_zoomOutButton);

    hboxLayout2 = new QHBoxLayout();
    hboxLayout2->setSpacing(6);
    hboxLayout2->setMargin(0);
    hboxLayout2->setObjectName(QString::fromUtf8("hboxLayout2"));
    textLabel1 = new QLabel(CP2PPI);
    textLabel1->setObjectName(QString::fromUtf8("textLabel1"));
    QSizePolicy sizePolicy9(static_cast<QSizePolicy::Policy>(1), static_cast<QSizePolicy::Policy>(1));
    sizePolicy9.setHorizontalStretch(0);
    sizePolicy9.setVerticalStretch(0);
    sizePolicy9.setHeightForWidth(textLabel1->sizePolicy().hasHeightForWidth());
    textLabel1->setSizePolicy(sizePolicy9);
    textLabel1->setMinimumSize(QSize(50, 0));
    textLabel1->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
    textLabel1->setWordWrap(false);

    hboxLayout2->addWidget(textLabel1);

    ZoomFactor = new QLCDNumber(CP2PPI);
    ZoomFactor->setObjectName(QString::fromUtf8("ZoomFactor"));
    ZoomFactor->setMinimumSize(QSize(50, 0));
    ZoomFactor->setFrameShape(QFrame::NoFrame);
    ZoomFactor->setSegmentStyle(QLCDNumber::Flat);

    hboxLayout2->addWidget(ZoomFactor);


    vboxLayout3->addLayout(hboxLayout2);


    vboxLayout2->addLayout(vboxLayout3);

    hboxLayout3 = new QHBoxLayout();
    hboxLayout3->setSpacing(6);
    hboxLayout3->setMargin(0);
    hboxLayout3->setObjectName(QString::fromUtf8("hboxLayout3"));
    _ringsCheckBox = new QCheckBox(CP2PPI);
    _ringsCheckBox->setObjectName(QString::fromUtf8("_ringsCheckBox"));
    _ringsCheckBox->setChecked(true);

    hboxLayout3->addWidget(_ringsCheckBox);

    _gridsCheckBox = new QCheckBox(CP2PPI);
    _gridsCheckBox->setObjectName(QString::fromUtf8("_gridsCheckBox"));

    hboxLayout3->addWidget(_gridsCheckBox);


    vboxLayout2->addLayout(hboxLayout3);

    vboxLayout4 = new QVBoxLayout();
    vboxLayout4->setSpacing(6);
    vboxLayout4->setMargin(0);
    vboxLayout4->setObjectName(QString::fromUtf8("vboxLayout4"));
    _colorButton = new QPushButton(CP2PPI);
    _colorButton->setObjectName(QString::fromUtf8("_colorButton"));

    vboxLayout4->addWidget(_colorButton);

    _ringColorButton = new QPushButton(CP2PPI);
    _ringColorButton->setObjectName(QString::fromUtf8("_ringColorButton"));

    vboxLayout4->addWidget(_ringColorButton);

    _saveButton = new QPushButton(CP2PPI);
    _saveButton->setObjectName(QString::fromUtf8("_saveButton"));

    vboxLayout4->addWidget(_saveButton);


    vboxLayout2->addLayout(vboxLayout4);

    _typeTab = new QTabWidget(CP2PPI);
    _typeTab->setObjectName(QString::fromUtf8("_typeTab"));
    QSizePolicy sizePolicy10(static_cast<QSizePolicy::Policy>(1), static_cast<QSizePolicy::Policy>(3));
    sizePolicy10.setHorizontalStretch(0);
    sizePolicy10.setVerticalStretch(0);
    sizePolicy10.setHeightForWidth(_typeTab->sizePolicy().hasHeightForWidth());
    _typeTab->setSizePolicy(sizePolicy10);
    _typeTab->setMinimumSize(QSize(100, 0));
    tab = new QWidget();
    tab->setObjectName(QString::fromUtf8("tab"));
    _typeTab->addTab(tab, QApplication::translate("CP2PPI", "Tab 1", 0, QApplication::UnicodeUTF8));

    vboxLayout2->addWidget(_typeTab);

    spacerItem = new QSpacerItem(232, 21, QSizePolicy::Minimum, QSizePolicy::Minimum);

    vboxLayout2->addItem(spacerItem);

    hboxLayout4 = new QHBoxLayout();
    hboxLayout4->setSpacing(6);
    hboxLayout4->setMargin(0);
    hboxLayout4->setObjectName(QString::fromUtf8("hboxLayout4"));
    textLabel1_3 = new QLabel(CP2PPI);
    textLabel1_3->setObjectName(QString::fromUtf8("textLabel1_3"));
    QSizePolicy sizePolicy11(static_cast<QSizePolicy::Policy>(1), static_cast<QSizePolicy::Policy>(1));
    sizePolicy11.setHorizontalStretch(0);
    sizePolicy11.setVerticalStretch(0);
    sizePolicy11.setHeightForWidth(textLabel1_3->sizePolicy().hasHeightForWidth());
    textLabel1_3->setSizePolicy(sizePolicy11);
    textLabel1_3->setMinimumSize(QSize(50, 0));
    textLabel1_3->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
    textLabel1_3->setWordWrap(false);

    hboxLayout4->addWidget(textLabel1_3);

    _azLCD = new QLCDNumber(CP2PPI);
    _azLCD->setObjectName(QString::fromUtf8("_azLCD"));
    QSizePolicy sizePolicy12(static_cast<QSizePolicy::Policy>(1), static_cast<QSizePolicy::Policy>(1));
    sizePolicy12.setHorizontalStretch(0);
    sizePolicy12.setVerticalStretch(0);
    sizePolicy12.setHeightForWidth(_azLCD->sizePolicy().hasHeightForWidth());
    _azLCD->setSizePolicy(sizePolicy12);
    _azLCD->setMinimumSize(QSize(50, 0));
    _azLCD->setFrameShape(QFrame::NoFrame);
    _azLCD->setFrameShadow(QFrame::Plain);
    _azLCD->setNumDigits(3);
    _azLCD->setSegmentStyle(QLCDNumber::Flat);

    hboxLayout4->addWidget(_azLCD);


    vboxLayout2->addLayout(hboxLayout4);

    hboxLayout5 = new QHBoxLayout();
    hboxLayout5->setSpacing(6);
    hboxLayout5->setMargin(0);
    hboxLayout5->setObjectName(QString::fromUtf8("hboxLayout5"));
    textLabel1_4 = new QLabel(CP2PPI);
    textLabel1_4->setObjectName(QString::fromUtf8("textLabel1_4"));
    QSizePolicy sizePolicy13(static_cast<QSizePolicy::Policy>(1), static_cast<QSizePolicy::Policy>(1));
    sizePolicy13.setHorizontalStretch(0);
    sizePolicy13.setVerticalStretch(0);
    sizePolicy13.setHeightForWidth(textLabel1_4->sizePolicy().hasHeightForWidth());
    textLabel1_4->setSizePolicy(sizePolicy13);
    textLabel1_4->setMinimumSize(QSize(50, 0));
    textLabel1_4->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
    textLabel1_4->setWordWrap(false);

    hboxLayout5->addWidget(textLabel1_4);

    _elLCD = new QLCDNumber(CP2PPI);
    _elLCD->setObjectName(QString::fromUtf8("_elLCD"));
    _elLCD->setMinimumSize(QSize(50, 0));
    _elLCD->setFrameShape(QFrame::NoFrame);
    _elLCD->setFrameShadow(QFrame::Plain);
    _elLCD->setNumDigits(3);
    _elLCD->setSegmentStyle(QLCDNumber::Flat);

    hboxLayout5->addWidget(_elLCD);


    vboxLayout2->addLayout(hboxLayout5);

    hboxLayout6 = new QHBoxLayout();
    hboxLayout6->setSpacing(6);
    hboxLayout6->setMargin(0);
    hboxLayout6->setObjectName(QString::fromUtf8("hboxLayout6"));
    _textIP = new QLabel(CP2PPI);
    _textIP->setObjectName(QString::fromUtf8("_textIP"));
    QSizePolicy sizePolicy14(static_cast<QSizePolicy::Policy>(1), static_cast<QSizePolicy::Policy>(1));
    sizePolicy14.setHorizontalStretch(0);
    sizePolicy14.setVerticalStretch(0);
    sizePolicy14.setHeightForWidth(_textIP->sizePolicy().hasHeightForWidth());
    _textIP->setSizePolicy(sizePolicy14);
    _textIP->setMinimumSize(QSize(50, 0));
    _textIP->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
    _textIP->setWordWrap(false);

    hboxLayout6->addWidget(_textIP);

    _textPort = new QLabel(CP2PPI);
    _textPort->setObjectName(QString::fromUtf8("_textPort"));
    QSizePolicy sizePolicy15(static_cast<QSizePolicy::Policy>(1), static_cast<QSizePolicy::Policy>(1));
    sizePolicy15.setHorizontalStretch(0);
    sizePolicy15.setVerticalStretch(0);
    sizePolicy15.setHeightForWidth(_textPort->sizePolicy().hasHeightForWidth());
    _textPort->setSizePolicy(sizePolicy15);
    _textPort->setMinimumSize(QSize(50, 0));
    _textPort->setWordWrap(false);

    hboxLayout6->addWidget(_textPort);


    vboxLayout2->addLayout(hboxLayout6);


    hboxLayout->addLayout(vboxLayout2);


    retranslateUi(CP2PPI);

    QSize size(1152, 865);
    size = size.expandedTo(CP2PPI->minimumSizeHint());
    CP2PPI->resize(size);


    _ppiStack->setCurrentIndex(1);


    QMetaObject::connectSlotsByName(CP2PPI);
    } // setupUi

    void retranslateUi(QWidget *CP2PPI)
    {
    CP2PPI->setWindowTitle(QApplication::translate("CP2PPI", "CP2PPI", 0, QApplication::UnicodeUTF8));
    _ppiS->setToolTip(QApplication::translate("CP2PPI", "knob", 0, QApplication::UnicodeUTF8));
    _ppiS->setWhatsThis(QApplication::translate("CP2PPI", "A knob that you can turn.", 0, QApplication::UnicodeUTF8));
    _ppiX->setToolTip(QApplication::translate("CP2PPI", "<html><head><meta name=\"qrichtext\" content=\"1\" /><style type=\"text/css\">\n"
"p, li { white-space: pre-wrap; }\n"
"</style></head><body style=\" font-family:'MS Shell Dlg 2'; font-size:8.25pt; font-weight:400; font-style:normal; text-decoration:none;\">\n"
"<p style=\"-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"></p></body></html>", 0, QApplication::UnicodeUTF8));
    _ppiX->setWhatsThis(QApplication::translate("CP2PPI", "A knob that you can turn.", 0, QApplication::UnicodeUTF8));
    _pauseButton->setText(QApplication::translate("CP2PPI", "Pause", 0, QApplication::UnicodeUTF8));
    _zoomInButton->setText(QApplication::translate("CP2PPI", "Zoom In", 0, QApplication::UnicodeUTF8));
    _resetButton->setText(QApplication::translate("CP2PPI", "Zoom Reset", 0, QApplication::UnicodeUTF8));
    _zoomOutButton->setText(QApplication::translate("CP2PPI", "Zoom Out", 0, QApplication::UnicodeUTF8));
    textLabel1->setText(QApplication::translate("CP2PPI", "Zoom:", 0, QApplication::UnicodeUTF8));
    _ringsCheckBox->setText(QApplication::translate("CP2PPI", "Rings", 0, QApplication::UnicodeUTF8));
    _gridsCheckBox->setText(QApplication::translate("CP2PPI", "Grids", 0, QApplication::UnicodeUTF8));
    _colorButton->setText(QApplication::translate("CP2PPI", "Background Color", 0, QApplication::UnicodeUTF8));
    _ringColorButton->setText(QApplication::translate("CP2PPI", "Ring and Grid Color", 0, QApplication::UnicodeUTF8));
    _saveButton->setText(QApplication::translate("CP2PPI", "Save Image", 0, QApplication::UnicodeUTF8));
    _typeTab->setTabText(_typeTab->indexOf(tab), QApplication::translate("CP2PPI", "Tab 1", 0, QApplication::UnicodeUTF8));
    textLabel1_3->setText(QApplication::translate("CP2PPI", "Azimuth", 0, QApplication::UnicodeUTF8));
    textLabel1_4->setText(QApplication::translate("CP2PPI", "Elevation", 0, QApplication::UnicodeUTF8));
    _textIP->setText(QApplication::translate("CP2PPI", "IP", 0, QApplication::UnicodeUTF8));
    _textPort->setText(QApplication::translate("CP2PPI", "Port", 0, QApplication::UnicodeUTF8));
    Q_UNUSED(CP2PPI);
    } // retranslateUi

};

namespace Ui {
    class CP2PPI: public Ui_CP2PPI {};
} // namespace Ui

#endif // UI_CP2PPI_H

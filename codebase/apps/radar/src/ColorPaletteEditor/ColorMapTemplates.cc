#include "ColorMapTemplates.hh"
#include "FlowLayout.hh"
#include "../HawkEye/ColorMap.hh"
#include "../HawkEye/ColorBar.hh"

#include <QPushButton>
#include <QApplication>
#include <QDesktopWidget>

ColorMapTemplates::ColorMapTemplates(QWidget *parent) :
    QDialog(parent)
{

   // add title to dialog
   const QRect availableGeometry = QApplication::desktop()->availableGeometry(this);
   resize(availableGeometry.width() / 3, availableGeometry.height() * 2 / 3); 
   setSizeGripEnabled(true); 
   // make the color map chooser

    //page = new QWidget;
    FlowLayout *flowLayout = new FlowLayout(this); // page);

    // make the color map picker

    ColorMap *cmap = new ColorMap(0.0, 100.0, "default");
    ColorBar *colorBar = new ColorBar(1, cmap); // , this);
    QPixmap *pixmap = colorBar->getPixmap();
    QLabel *cmapLabel = new QLabel();
    int w = cmapLabel->width();
    int h = cmapLabel->height();
    cmapLabel->clear();
    cmapLabel->setPixmap(pixmap->scaled(w/2,h)); // ,Qt::KeepAspectRatio));
    string name = "default"; // cmap->getName();
    cmapLabel->setToolTip(QString::fromStdString(name));
    flowLayout->addWidget(cmapLabel);

    cmap = new ColorMap(0.0, 100.0, "rainbow");
    colorBar = new ColorBar(1, cmap); // , this);
    pixmap = colorBar->getPixmap();
    cmapLabel = new QLabel();
    cmapLabel->clear();
    cmapLabel->setPixmap(pixmap->scaled(w/2,h));
    name = "rainbow"; //cmap->getName();
    cmapLabel->setToolTip(QString::fromStdString(name));
    flowLayout->addWidget(cmapLabel);

    cmap = new ColorMap(0.0, 100.0, "eldoraDbz");
    colorBar = new ColorBar(1, cmap); // , this);
    pixmap = colorBar->getPixmap();
    cmapLabel = new QLabel();
    cmapLabel->clear();
    cmapLabel->setPixmap(pixmap->scaled(w/2,h));
    name = "eldoraDbz"; // cmap->getName();
    cmapLabel->setToolTip(QString::fromStdString(name));
    flowLayout->addWidget(cmapLabel);

    cmap = new ColorMap(0.0, 100.0, "spolDbz");
    colorBar = new ColorBar(1, cmap); // , this);
    pixmap = colorBar->getPixmap();
    cmapLabel = new QLabel();
    cmapLabel->clear();
    cmapLabel->setPixmap(pixmap->scaled(w/2,h));
    name = "spolDbz"; // cmap->getName();
    cmapLabel->setToolTip(QString::fromStdString(name));
    flowLayout->addWidget(cmapLabel);

    cmap = new ColorMap(0.0, 100.0, "eldoraVel");
    colorBar = new ColorBar(1, cmap); // , this);
    pixmap = colorBar->getPixmap();
    cmapLabel = new QLabel();
    cmapLabel->clear();
    cmapLabel->setPixmap(pixmap->scaled(w/2,h));
    name = "eldoraVel"; // cmap->getName();
    cmapLabel->setToolTip(QString::fromStdString(name));
    flowLayout->addWidget(cmapLabel);

    cmap = new ColorMap(0.0, 100.0, "spolVel");
    colorBar = new ColorBar(1, cmap); // , this);
    pixmap = colorBar->getPixmap();
    cmapLabel = new QLabel();
    cmapLabel->clear();
    cmapLabel->setPixmap(pixmap->scaled(w/2,h));
    name = "spolVel"; // cmap->getName();
    cmapLabel->setToolTip(QString::fromStdString(name));
    flowLayout->addWidget(cmapLabel);

    cmap = new ColorMap(0.0, 100.0, "spolDiv");
    colorBar = new ColorBar(1, cmap); // , this);
    pixmap = colorBar->getPixmap();
    cmapLabel = new QLabel();
    cmapLabel->clear();
    cmapLabel->setPixmap(pixmap->scaled(w/2,h));
    name = "spolDiv"; // cmap->getName();
    cmapLabel->setToolTip(QString::fromStdString(name));
    flowLayout->addWidget(cmapLabel);

}

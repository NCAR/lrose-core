#include "ColorMapTemplates.hh"
#include "FlowLayout.hh"
#include "../HawkEye/ColorBar.hh"
#include "toolsa/LogStream.hh"

#include <QPushButton>
#include <QApplication>
#include <QDesktopWidget>
#include <QFileDialog>

ColorMapTemplates *ColorMapTemplates::_instance = NULL;

ColorMapTemplates *ColorMapTemplates::getInstance(QWidget *parent) {
  if (_instance == NULL) 
    _instance = new ColorMapTemplates(parent);
  
  return _instance;
}

ColorMapTemplates::ColorMapTemplates(QWidget *parent) :
    QDialog(parent)
{

   // add title to dialog
   //const QRect availableGeometry = QApplication::desktop()->availableGeometry(this);
   //resize(availableGeometry.width() / 3, availableGeometry.height() * 2 / 3); 
   //setSizeGripEnabled(true); 
   // make the color map chooser

    //page = new QWidget;
    FlowLayout *flowLayout = new FlowLayout(this); // page);

    int wd = 8;
    int hd = 3;

    // make the color map picker

    _defaultColorMap = new ColorMap(0.0, 100.0, "default");
    ColorBar *colorBar = new ColorBar(1, _defaultColorMap);
    colorBar->setAnnotationOff();
    QPixmap *pixmap = colorBar->getPixmap();
    _defaultColorMapLabel = new ClickableLabel();
    int w = _defaultColorMapLabel->width();
    int h = _defaultColorMapLabel->height();
    _defaultColorMapLabel->clear();
    _defaultColorMapLabel->setPixmap(pixmap->scaled(w/wd,h/hd)); // ,Qt::KeepAspectRatio));
    string name = "default"; // cmap->getName();
    _defaultColorMapLabel->setToolTip(QString::fromStdString(name));
    flowLayout->addWidget(_defaultColorMapLabel);

    resize(w*2, h/2); 
    setSizeGripEnabled(true); 


    //    QRect rectangle(0, 0, w*70, h);
    //flowLayout->setGeometry(rectangle);

    //ColorMap *cmap;
    //ClickableLabel *cmapLabel;

    _rainbowColorMap = new ColorMap(0.0, 100.0, "rainbow");
    colorBar = new ColorBar(1, _rainbowColorMap);
    colorBar->setAnnotationOff();
    pixmap = colorBar->getPixmap();
    _rainbowColorMapLabel = new ClickableLabel();
    _rainbowColorMapLabel->clear();
    _rainbowColorMapLabel->setPixmap(pixmap->scaled(w/wd,h/hd));
    name = "rainbow"; //cmap->getName();
    _rainbowColorMapLabel->setToolTip(QString::fromStdString(name));
    flowLayout->addWidget(_rainbowColorMapLabel);

    _eldoraDbzColorMap = new ColorMap(0.0, 100.0, "eldoraDbz");
    colorBar = new ColorBar(1, _eldoraDbzColorMap);
    colorBar->setAnnotationOff();
    pixmap = colorBar->getPixmap();
    _eldoraDbzColorMapLabel = new ClickableLabel();
    _eldoraDbzColorMapLabel->clear();
    _eldoraDbzColorMapLabel->setPixmap(pixmap->scaled(w/wd,h/hd));
    name = "eldoraDbz"; // cmap->getName();
    _eldoraDbzColorMapLabel->setToolTip(QString::fromStdString(name));
    flowLayout->addWidget(_eldoraDbzColorMapLabel);

    _spolDbzColorMap = new ColorMap(0.0, 100.0, "spolDbz");
    colorBar = new ColorBar(1, _spolDbzColorMap);
    colorBar->setAnnotationOff();
    pixmap = colorBar->getPixmap();
    _spolDbzColorMapLabel = new ClickableLabel();
    _spolDbzColorMapLabel->clear();
    _spolDbzColorMapLabel->setPixmap(pixmap->scaled(w/wd,h/hd));
    name = "spolDbz"; // cmap->getName();
    _spolDbzColorMapLabel->setToolTip(QString::fromStdString(name));
    flowLayout->addWidget(_spolDbzColorMapLabel);

    _eldoraVelColorMap = new ColorMap(0.0, 100.0, "eldoraVel");
    colorBar = new ColorBar(1, _eldoraVelColorMap);
    colorBar->setAnnotationOff();
    pixmap = colorBar->getPixmap();
    _eldoraVelColorMapLabel = new ClickableLabel();
    _eldoraVelColorMapLabel->clear();
    _eldoraVelColorMapLabel->setPixmap(pixmap->scaled(w/wd,h/hd));
    name = "eldoraVel"; // cmap->getName();
    _eldoraVelColorMapLabel->setToolTip(QString::fromStdString(name));
    flowLayout->addWidget(_eldoraVelColorMapLabel);

    _spolVelColorMap = new ColorMap(0.0, 100.0, "spolVel");
    colorBar = new ColorBar(1, _spolVelColorMap);
    colorBar->setAnnotationOff();
    pixmap = colorBar->getPixmap();
    _spolVelColorMapLabel = new ClickableLabel();
    _spolVelColorMapLabel->clear();
    _spolVelColorMapLabel->setPixmap(pixmap->scaled(w/wd,h/hd));
    name = "spolVel"; // cmap->getName();
    _spolVelColorMapLabel->setToolTip(QString::fromStdString(name));
    flowLayout->addWidget(_spolVelColorMapLabel);

    _spolDivColorMap = new ColorMap(0.0, 100.0, "spolDiv");
    colorBar = new ColorBar(1, _spolDivColorMap);
    colorBar->setAnnotationOff();
    pixmap = colorBar->getPixmap();
    _spolDivColorMapLabel = new ClickableLabel();
    _spolDivColorMapLabel->clear();
    _spolDivColorMapLabel->setPixmap(pixmap->scaled(w/wd,h/hd));
    name = "spolDiv"; // cmap->getName();
    _spolDivColorMapLabel->setToolTip(QString::fromStdString(name));
    flowLayout->addWidget(_spolDivColorMapLabel);

    // TODO: get the list of known color maps from the authority
    // TODO: allow custom defined color maps; save them to a special directory
    // and read from that directory as well.

    // add the connections
    // We'll need the coordinates of the label, or uniquely name the color maps
    // connect(cmapLabel, &ClickableLabel::clicked, this, &ParameterColorDialog::pickColorPalette);


    QPushButton *_importColorMapButton = new QPushButton("Import ColorMap");
    flowLayout->addWidget(_importColorMapButton);

    setLayout(flowLayout);

    connect(_defaultColorMapLabel,   &ClickableLabel::clicked, this, &ColorMapTemplates::defaultClicked);
    connect(_rainbowColorMapLabel,   &ClickableLabel::clicked, this, &ColorMapTemplates::rainbowClicked);
    connect(_eldoraDbzColorMapLabel, &ClickableLabel::clicked, this, &ColorMapTemplates::eldoraDbzClicked);
    connect(_eldoraVelColorMapLabel, &ClickableLabel::clicked, this, &ColorMapTemplates::eldoraVelClicked);
    connect(_spolVelColorMapLabel,   &ClickableLabel::clicked, this, &ColorMapTemplates::spolVelClicked);
    connect(_spolDivColorMapLabel,   &ClickableLabel::clicked, this, &ColorMapTemplates::spolDivClicked);
    connect(_spolDbzColorMapLabel,   &ClickableLabel::clicked, this, &ColorMapTemplates::spolDbzClicked);

    connect(_importColorMapButton,   &QPushButton::clicked, this, &ColorMapTemplates::importColorMap);

}


ColorMapTemplates::~ColorMapTemplates() {
  LOG(DEBUG) << "entry";
  // clean up ...
  /* delete each colorMap; Hmm, maybe each color map should be in a vector? */
  LOG(DEBUG) << "exit";
}


// What to do:  send pointer? or make copy and send a copy?  Which is better for memory management?
// send a copy; delete all pointers on Dialog close; pass along responsibility to next class.
void ColorMapTemplates::defaultClicked() {
  LOG(DEBUG) << "entry";
  emit newColorPaletteSelected("default"); 
  LOG(DEBUG) << "exit";
}

void ColorMapTemplates::rainbowClicked() {
  LOG(DEBUG) << "entry";
  emit newColorPaletteSelected("rainbow");
  LOG(DEBUG) << "exit";
}

void ColorMapTemplates::eldoraDbzClicked() {
  LOG(DEBUG) << "entry";
  emit newColorPaletteSelected("eldoraDbz");
  LOG(DEBUG) << "exit";
}

void ColorMapTemplates::eldoraVelClicked() {
  LOG(DEBUG) << "entry";
  emit newColorPaletteSelected("eldoraVel");
  LOG(DEBUG) << "exit";
}

void ColorMapTemplates::spolVelClicked() {
  LOG(DEBUG) << "entry";
  emit newColorPaletteSelected("spolVel");
  LOG(DEBUG) << "exit";
}

void ColorMapTemplates::spolDivClicked() {
  LOG(DEBUG) << "entry";
  emit newColorPaletteSelected("spolDiv");
  LOG(DEBUG) << "exit";
}

void ColorMapTemplates::spolDbzClicked() {
  LOG(DEBUG) << "entry";
  emit newColorPaletteSelected("spolDbz");
  LOG(DEBUG) << "exit";
}

void ColorMapTemplates::importedMapClicked() {
  LOG(DEBUG) << "entry";
   // e.g. check with member variable _foobarButton
   //QObject* obj = sender();
   //if( obj == _foobarButton )
   //{ 
   //   ...
   //}

   // e.g. casting to the class you know its connected with
   ClickableLabel* label = qobject_cast<ClickableLabel*>(sender());
   if( label != NULL ) 
   { 
      string colorScaleName = label->toolTip().toStdString();
      LOG(DEBUG) << "just clicked " << colorScaleName;
   
      emit newColorPaletteSelected(colorScaleName);
   }
  LOG(DEBUG) << "exit";
}
/*
void ColorMapTemplates::importColorMapClicked(QString &text) {
  LOG(DEBUG) << "entry";
 // emit newColorPaletteSelected("???");
  LOG(DEBUG) << "exit";
}
*/

void ColorMapTemplates::importColorMap() {
  QString fileNameQ = QFileDialog::getOpenFileName(this,
    tr("Import ColorMap"), "../share/color_scales", tr("ColorMap Files (*.colors)"));
  string fileName = fileNameQ.toStdString();
  LOG(DEBUG) << "fileName is " << fileName;

    int wd = 2;
    int hd = 1;

//  ColorMap(const std::string &file_path, bool debug = false);

    ColorMap *newMap = new ColorMap(fileName, false);
    ColorBar *colorBar = new ColorBar(1, newMap);
    colorBar->setAnnotationOff();
    QPixmap *pixmap = colorBar->getPixmap();
    ClickableLabel *newMapLabel = new ClickableLabel();
    newMapLabel->clear();
    int w = _defaultColorMapLabel->width();
    int h = _defaultColorMapLabel->height();
    newMapLabel->setPixmap(pixmap->scaled(w/wd,h/hd));
    newMapLabel->setToolTip(fileNameQ);

    layout()->addWidget(newMapLabel);

    // connect newly imported color scale to slot with color scale name
    connect(newMapLabel, &ClickableLabel::clicked, this, &ColorMapTemplates::importedMapClicked);

    // save the ColorMap in the list of imports
    _imports[fileName] = newMap;

}

ColorMap *ColorMapTemplates::getColorMap(string name) {
  std::map<std::string, ColorMap *>::iterator it;
  it = _imports.find(name);
  if (it == _imports.end()) {
    LOG(DEBUG) << "color map not found for " << name;
    return NULL;
  } else {
    return it->second;
  }
}

/*

ClickableLabel *getInternalColorMapLabel(string name) {
    int wd = 8;
    int hd = 3;

    // make the color map picker

    _defaultColorMap = new ColorMap(0.0, 100.0, "default");
    ColorBar *colorBar = new ColorBar(1, _defaultColorMap);
    colorBar->setAnnotationOff();
    QPixmap *pixmap = colorBar->getPixmap();
    _defaultColorMapLabel = new ClickableLabel();
    int w = _defaultColorMapLabel->width();
    int h = _defaultColorMapLabel->height();
    _defaultColorMapLabel->clear();
    _defaultColorMapLabel->setPixmap(pixmap->scaled(w/wd,h/hd)); // ,Qt::KeepAspectRatio));
    string name = "default"; // cmap->getName();
    _defaultColorMapLabel->setToolTip(QString::fromStdString(name));
    flowLayout->addWidget(_defaultColorMapLabel);
}
*/

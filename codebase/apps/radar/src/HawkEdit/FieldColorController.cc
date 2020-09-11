
#include "toolsa/LogStream.hh"
#include "FieldColorController.hh"
#include "DisplayField.hh"
#include "ColorMap.hh"
#include "ColorBar.hh"

FieldColorController::FieldColorController(ParameterColorView *parameterColorView,
					   DisplayFieldModel *displayFieldModel)
{
  _view = parameterColorView;
  _model = displayFieldModel;
  // connections ...
  connect(_view, SIGNAL(getColorMap(string)), this, SLOT(getColorMap(string)));
  connect(_view, SIGNAL(getBackgroundColor()), this, SLOT(getBackgroundColor()));
  //connect(_view, SIGNAL(setV()), this, SLOT(setV()));

  connect(_view, SIGNAL(colorMapMaxChanged(double)), this, SLOT(colorMapMaxChanged(double)));
  connect(_view, SIGNAL(colorMapMinChanged(double)), this, SLOT(colorMapMinChanged(double)));

  connect(_view, SIGNAL(pickColorPaletteRequest()), this, SLOT(pickColorPaletteRequest()));
  connect(_view, SIGNAL(gridColorChanged(QColor)), this, SLOT(newGridColorSelected(QColor)));
  connect(_view, SIGNAL(backgroundColorChanged(QColor)), this, SLOT(newBackgroundColorSelected(QColor)));
  //connect(this, SIGNAL(updateEvent(vector<string>, string)),
  // _view, SLOT(updateEvent(vector<string>, string)));

  // TODO replot is messed up; try naming more specific
  connect(_view, SIGNAL(replotFieldColorMapChanges()), this, SLOT(modelChanged()));
  vector<string> fieldNames = _model->getFieldNames();
  string selectedField = _model->getSelectedFieldName();
  // ColorMap colorMap = _model->getColorMap(selectedField);
  _view->updateEvent(fieldNames, selectedField);

}

FieldColorController::~FieldColorController() {

  // disconnect listeners

  //TODO:  clean up memory
  //delete *_view;
  //delete *_model;
}

void FieldColorController::startUp()
{
  _view->exec();
}

QColor FieldColorController::_stringToQColor(string colorName) {

  QString colorNameQ = QString::fromStdString(colorName);
  if (QColor::isValidColor(colorNameQ)) {
    QColor color(colorNameQ);
    return color;
  }
  else {
    throw "Cannot recognize color";
  }
}

// in response to "Save" from View
void FieldColorController::modelChanged() // string fieldName) // , ColorMap newColorMap) 
{
  LOG(DEBUG) << "enter"; 

  string selectedField = _view->getSelectedFieldName();

  LOG(DEBUG) << "selected field is " << selectedField;

  // get changes from model
  /*
  if (_model->colorMapChanged(fieldName)) {
  }
  if (_model->backgroundChanged(fieldName)) {
  }
  */
  ColorMap *newColorMap = _model->getColorMap(selectedField);
  string color;
  color = _model->getGridColor();
  LOG(DEBUG) << "grid:" << color;
  QColor gridColor = _stringToQColor(color);
  color = _model->getEmphasisColor();
  QColor emphasisColor = _stringToQColor(color);
  color = _model->getAnnotationColor();
  QColor annotationColor = _stringToQColor(color);
  color = _model->getBackgroundColor();
  QColor backgroundColor = _stringToQColor(color);

  //  fieldName should be current working fieldName & colorMap
  LOG(DEBUG) << "emit colorMapRedefineSent";
  LOG(DEBUG) << "grid:" << gridColor.name().toStdString();
  emit colorMapRedefineSent(selectedField, *newColorMap, gridColor, emphasisColor,
			    annotationColor, backgroundColor);
  LOG(DEBUG) << "exit";
}

void FieldColorController::getColorMap(string fieldName) 
{
  // get info from model
  ColorMap *colorMap = _model->getColorMap(fieldName);
  
   _view->colorMapProvided(fieldName, colorMap);
}

void FieldColorController::getGridColor() 
{
  // get info from model
  string colorName = _model->getGridColor();
  QColor color = _stringToQColor(colorName);
  _view->gridColorProvided(color);

  /*  QString colorNameQ = QString::fromStdString(colorName);
  if (QColor::isValidColor(colorNameQ)) {
    QColor color(colorNameQ);
    _view->gridColorProvided(color);
  }
  else {
    throw "Cannot recognize color";
  }
  */
}

void FieldColorController::getEmphasisColor() 
{
  // get info from model
  string colorName = _model->getEmphasisColor();
  QString colorNameQ = QString::fromStdString(colorName);
  if (QColor::isValidColor(colorNameQ)) {
    QColor color(colorNameQ);
    _view->emphasisColorProvided(color);
  }
  else {
    throw "Cannot recognize color";
  }
}

void FieldColorController::getAnnotationColor() 
{
  // get info from model
  string colorName = _model->getAnnotationColor();
  QString colorNameQ = QString::fromStdString(colorName);
  if (QColor::isValidColor(colorNameQ)) {
    QColor color(colorNameQ);
    _view->annotationColorProvided(color);
  }
  else {
    throw "Cannot recognize color";
  }
}

void FieldColorController::getBackgroundColor() 
{
  // get info from model
  string colorName = _model->getBackgroundColor();
  QString colorNameQ = QString::fromStdString(colorName);
  if (QColor::isValidColor(colorNameQ)) {
    QColor color(colorNameQ);
    _view->backgroundColorProvided(color);
  }
  else {
    throw "Cannot recognize color";
  }
}

void FieldColorController::colorMapMaxChanged(double newValue) {

  string affectedFieldName = _view->getSelectedFieldName();
  ColorMap *colorMap =_model->colorMapMaxChanged(affectedFieldName, newValue);

  //  send new working colorMap to _view
  _view->colorMapProvided("", colorMap); 
}

void FieldColorController::colorMapMinChanged(double newValue) {

  string affectedFieldName = _view->getSelectedFieldName();
  ColorMap *colorMap =_model->colorMapMinChanged(affectedFieldName, newValue);

  //  send new working colorMap to _view
  _view->colorMapProvided("", colorMap); 
}

void FieldColorController::pickColorPaletteRequest()
{
  _colorMapTemplates = _colorMapTemplates->getInstance(_view);
  //  colorMapTemplates.init();
  
  connect(_colorMapTemplates, SIGNAL(newColorPaletteSelected(string)),
	  this, SLOT(newColorPaletteSelected(string)));
  _colorMapTemplates->exec();
}

void FieldColorController::newColorPaletteSelected(string newColorMapName) {

  string editField = _view->getSelectedFieldName();
  _model->colorMapChanged(editField, newColorMapName);
  getColorMap(editField);
  _colorMapTemplates->close();
  //  _view->colorMapProvided("", newColorMap);

}

//void FieldColorController::newColorPaletteSelected(ColorMap *newColorMap) {

//  string editField = _view->getSelectedFieldName();
//  _model->colorMapChanged(editField, newColorMap);
//  getColorMap(editField);
//  _colorMapTemplates->close();
  //  _view->colorMapProvided("", newColorMap);

//}

void FieldColorController::newGridColorSelected(QColor newColor) {
  LOG(DEBUG) << "enter ";
  string color = newColor.name().toStdString();
  LOG(DEBUG) << color;
  _model->setGridColor(color);
  emit gridColorSet(newColor);
  LOG(DEBUG) << "exit";
}

void FieldColorController::newEmphasisColorSelected(QColor newColor) {
  LOG(DEBUG) << "enter ";
  string color = newColor.name().toStdString();
  LOG(DEBUG) << color;
  _model->setEmphasisColor(color);
  emit emphasisColorSet(newColor);
  LOG(DEBUG) << "exit";
}

void FieldColorController::newAnnotationColorSelected(QColor newColor) {
  LOG(DEBUG) << "enter ";
  string color = newColor.name().toStdString();
  LOG(DEBUG) << color;
  _model->setAnnotationColor(color);
  emit annotationColorSet(newColor);
  LOG(DEBUG) << "exit";
}

void FieldColorController::newBackgroundColorSelected(QColor newColor) {
  LOG(DEBUG) << "enter ";
  string color = newColor.name().toStdString();
  LOG(DEBUG) << color;
  _model->setBackgroundColor(color);
  emit backgroundColorSet(newColor);
  LOG(DEBUG) << "exit";
}



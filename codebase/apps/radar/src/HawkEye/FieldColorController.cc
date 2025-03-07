
#include "toolsa/LogStream.hh"
#include "FieldColorController.hh"
#include "DisplayField.hh"
#include <qtplot/ColorMap.hh>
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
  string selectedField = _model->getSelectedField();
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
  if (_isValidColorName(colorNameQ)) {
    QColor color(colorNameQ);
    return color;
  }
  else {
    throw "Cannot recognize color";
  }
}

// in response to Replot from View
void FieldColorController::modelChanged() // string fieldName) // , ColorMap newColorMap) 
{
  LOG(DEBUG_VERBOSE) << "enter"; 

  string selectedField = _model->getSelectedField();

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
  LOG(DEBUG_VERBOSE) << "grid:" << color;
  QColor gridColor = _stringToQColor(color);
  color = _model->getEmphasisColor();
  QColor emphasisColor = _stringToQColor(color);
  color = _model->getAnnotationColor();
  QColor annotationColor = _stringToQColor(color);
  color = _model->getBackgroundColor();
  QColor backgroundColor = _stringToQColor(color);

  //  fieldName should be current working fieldName & colorMap
  LOG(DEBUG_VERBOSE) << "emit colorMapRedefineSent";
  LOG(DEBUG_VERBOSE) << "grid:" << gridColor.name().toStdString();
  emit colorMapRedefineSent(selectedField, *newColorMap, gridColor, emphasisColor,
			    annotationColor, backgroundColor);
  LOG(DEBUG_VERBOSE) << "exit";
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
  if (_isValidColorName(colorNameQ)) {
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
  if (_isValidColorName(colorNameQ)) {
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
  if (_isValidColorName(colorNameQ)) {
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
  if (_isValidColorName(colorNameQ)) {
    QColor color(colorNameQ);
    _view->backgroundColorProvided(color);
  }
  else {
    throw "Cannot recognize color";
  }
}

void FieldColorController::colorMapMaxChanged(double newValue) {

  ColorMap *colorMap =_model->colorMapMaxChanged(newValue);

  //  send new working colorMap to _view
  _view->colorMapProvided("", colorMap); 
}

void FieldColorController::colorMapMinChanged(double newValue) {

  ColorMap *colorMap =_model->colorMapMinChanged(newValue);

  //  send new working colorMap to _view
  _view->colorMapProvided("", colorMap); 
}

void FieldColorController::pickColorPaletteRequest()
{
  _colorMapTemplates = new ColorMapTemplates(_view);
  //  colorMapTemplates.init();
  
  connect(_colorMapTemplates, SIGNAL(newColorPaletteSelected(string)),
	  this, SLOT(newColorPaletteSelected(string)));
  _colorMapTemplates->exec();
}

void FieldColorController::newColorPaletteSelected(string newColorMapName) {

  _model->colorMapChanged(newColorMapName);
  getColorMap(_model->getSelectedField());
  _colorMapTemplates->close();
  //  _view->colorMapProvided("", newColorMap);

}

void FieldColorController::newGridColorSelected(QColor newColor) {
  LOG(DEBUG_VERBOSE) << "enter ";
  string color = newColor.name().toStdString();
  LOG(DEBUG_VERBOSE) << color;
  _model->setGridColor(color);
  emit gridColorSet(newColor);
  LOG(DEBUG_VERBOSE) << "exit";
}

void FieldColorController::newEmphasisColorSelected(QColor newColor) {
  LOG(DEBUG_VERBOSE) << "enter ";
  string color = newColor.name().toStdString();
  LOG(DEBUG_VERBOSE) << color;
  _model->setEmphasisColor(color);
  emit emphasisColorSet(newColor);
  LOG(DEBUG_VERBOSE) << "exit";
}

void FieldColorController::newAnnotationColorSelected(QColor newColor) {
  LOG(DEBUG_VERBOSE) << "enter ";
  string color = newColor.name().toStdString();
  LOG(DEBUG_VERBOSE) << color;
  _model->setAnnotationColor(color);
  emit annotationColorSet(newColor);
  LOG(DEBUG_VERBOSE) << "exit";
}

void FieldColorController::newBackgroundColorSelected(QColor newColor) {
  LOG(DEBUG_VERBOSE) << "enter ";
  string color = newColor.name().toStdString();
  LOG(DEBUG_VERBOSE) << color;
  _model->setBackgroundColor(color);
  emit backgroundColorSet(newColor);
  LOG(DEBUG_VERBOSE) << "exit";
}

/////////////////////////////////////////////////////
// check that color name is valid

bool FieldColorController::_isValidColorName(QString colorName)
{
#if QT_VERSION >= 0x060000
  return QColor::isValidColorName(colorName);
#else
  QColor color;
  color.setNamedColor(colorName);
  return color.isValid();
#endif
}

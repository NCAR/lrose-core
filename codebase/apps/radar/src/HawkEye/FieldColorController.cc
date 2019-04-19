
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
  //connect(_view, SIGNAL(setV()), this, SLOT(setV()));

  connect(_view, SIGNAL(colorMapMaxChanged(double)), this, SLOT(colorMapMaxChanged(double)));
  connect(_view, SIGNAL(colorMapMinChanged(double)), this, SLOT(colorMapMinChanged(double)));

  connect(_view, SIGNAL(pickColorPaletteRequest()), this, SLOT(pickColorPaletteRequest()));

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

void FieldColorController::modelChanged() // string fieldName) // , ColorMap newColorMap) 
{
  LOG(DEBUG) << "enter"; 

  string selectedField = _model->getSelectedField();

  // get changes from model
  /*
  if (_model->colorMapChanged(fieldName)) {
  }
  if (_model->backgroundChanged(fieldName)) {
  }
  */
  ColorMap *newColorMap = _model->getColorMap(selectedField);
  //  fieldName should be current working fieldName & colorMap
  LOG(DEBUG) << "emit colorMapRedefineSent";
  emit colorMapRedefineSent(selectedField, *newColorMap);
  LOG(DEBUG) << "exit";
}

void FieldColorController::getColorMap(string fieldName) 
{
  // get info from model
  ColorMap *colorMap = _model->getColorMap(fieldName);
  
   _view->colorMapProvided(fieldName, colorMap);
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


/*
void FieldColorController::getFieldNames() {
  vector<string> fieldNames = _model->getFieldNames();
  _view->hereAreFieldNames(fieldNames);
}
*/
//map<string, string> colorMap???


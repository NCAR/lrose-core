
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

  //connect(this, SIGNAL(updateEvent(vector<string>, string)),
  // _view, SLOT(updateEvent(vector<string>, string)));

  // TODO replot is messed up; try naming more specific
  //  connect(_view, SIGNAL(replot()), this, SLOT(modelChanged));
  vector<string> fieldNames = _model->getFieldNames();
  string selectedField = _model->getSelectedField();
  // ColorMap colorMap = _model->getColorMap(selectedField);
  _view->updateEvent(fieldNames, selectedField);

  _view->exec();

}

FieldColorController::~FieldColorController() {

  // disconnect listeners

  //TODO:  clean up memory
  //delete *_view;
  //delete *_model;
}

void FieldColorController::modelChanged(string fieldName) // , ColorMap newColorMap) 
{
  // TODO: need to fix this up a bit ...
  // TODO: Save state somehow in model ...
  // get changes from model
  if (_model->colorMapChanged(fieldName)) {
  }
  if (_model->backgroundChanged(fieldName)) {
  }
  //  emit colorMapRedefined(fieldName, newColorMap);

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

/*
void FieldColorController::getFieldNames() {
  vector<string> fieldNames = _model->getFieldNames();
  _view->hereAreFieldNames(fieldNames);
}
*/
//map<string, string> colorMap???


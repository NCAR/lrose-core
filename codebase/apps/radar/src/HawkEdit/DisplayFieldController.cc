
#include "toolsa/LogStream.hh"
#include "DisplayFieldController.hh"
//#include "DisplayField.hh"
#include "ColorMap.hh"
#include "ColorBar.hh"


DisplayFieldController::DisplayFieldController(DisplayFieldModel *model)
{
  LOG(DEBUG) << "enter";
  _model = model;
  LOG(DEBUG) << "exit";
}

DisplayFieldController::~DisplayFieldController() {
  //  delete _model;
}

void DisplayFieldController::addField(DisplayField *newField) {
  _model->addField(newField);
}

vector<string>  DisplayFieldController::getFieldNames() {
  return _model->getFieldNames();
} 

size_t DisplayFieldController::getNFields() {
  return _model->getNFields();
}  

DisplayField *DisplayFieldController::getField(size_t fieldIndex) {
  return _model->getField(fieldIndex);
}  

DisplayField *DisplayFieldController::getField(string fieldName) {
  return _model->getField(fieldName);
}  

size_t DisplayFieldController::getFieldIndex(string fieldName) {
  return _model->getFieldIndex(fieldName);
}  

string DisplayFieldController::getFieldAlias(string fieldName) {
  return _model->getFieldAlias(fieldName);
} 


string DisplayFieldController::getSelectedFieldName() {
  //  DisplayField *selectedField = model->getSelectedField();
  //  return selectedField->getName();
  return _model->getSelectedFieldName();
} 

DisplayField *DisplayFieldController::getSelectedField() {
  return _model->getSelectedField();
} 

size_t DisplayFieldController::getSelectedFieldNum() {
  return _model->getSelectedFieldNum();
} 


void DisplayFieldController::setSelectedField(string fieldName) {
  LOG(DEBUG) << "enter " << fieldName;
  _model->setSelectedField(fieldName);
  LOG(DEBUG) << "exit";
} 


void DisplayFieldController::setSelectedField(size_t fieldIndex) {
  _model->setSelectedField(fieldIndex);
} 

DisplayField *DisplayFieldController::getFiltered(size_t ifield, int buttonRow)
{
  return _model->getFiltered(ifield, buttonRow);
}

/*
bool DisplayFieldController::getChanges() {

   //show();
   return false;

}
*/

void DisplayFieldController::setForLocationClicked(double value, const string &text) {
  _model->setForLocationClicked(value, text);
}

void DisplayFieldController::setForLocationClicked(string fieldName, double value, const string &text) {
  _model->setForLocationClicked(fieldName, value, text);
}

ColorMap *DisplayFieldController::getColorMap(size_t fieldIndex) {
  string fieldName = _model->getFieldName(fieldIndex);
  return _model->getColorMap(fieldName);
}

ColorMap *DisplayFieldController::getColorMap(string fieldName) {
  return _model->getColorMap(fieldName);
}

void DisplayFieldController::setColorMap(string fieldName, ColorMap *newColorMap) {
  
  LOG(DEBUG) << "entry " << fieldName;
  _model->setColorMap(fieldName, newColorMap);
  LOG(DEBUG) << "exit";
}

void DisplayFieldController::saveColorMap(string fieldName, ColorMap *newColorMap) {
  
  LOG(DEBUG) << "entry " << fieldName;
  _model->saveColorMap(fieldName, newColorMap);
  LOG(DEBUG) << "exit";
}


ColorMap *DisplayFieldController::colorMapMaxChanged(double newValue) {
  return _model->colorMapMaxChanged(newValue);
}

ColorMap *DisplayFieldController::colorMapMinChanged(double newValue) {
  return _model->colorMapMinChanged(newValue);
}

void DisplayFieldController::setColorMapMinMax(string fieldName, double min, double max) {
  LOG(DEBUG) << "entry " << fieldName << ", min=" << min << ", max=" << max;
  _model->setColorMapMinMax(fieldName, min, max);
  LOG(DEBUG) << "exit";
}


void DisplayFieldController::colorMapChanged(string newColorMapName) {
  LOG(DEBUG) << "enter";
  _model->colorMapChanged(newColorMapName);
  LOG(DEBUG) << "exit";
}

bool DisplayFieldController::backgroundChanged(string fieldName) {
  return _model->backgroundChanged(fieldName);
}

string DisplayFieldController::getGridColor() {
  return _model->getGridColor();
}

void DisplayFieldController::setGridColor(string colorName) {
  LOG(DEBUG) << "enter " << colorName;
  _model->setGridColor(colorName);
  LOG(DEBUG) << "exit";
}

string DisplayFieldController::getEmphasisColor() {
  return _model->getEmphasisColor();
}

void DisplayFieldController::setEmphasisColor(string colorName) {
  _model->setEmphasisColor(colorName);
}

string DisplayFieldController::getAnnotationColor() {
  return _model->getAnnotationColor();
}

void DisplayFieldController::setAnnotationColor(string colorName) {
  _model->setAnnotationColor(colorName);
}

string DisplayFieldController::getBackgroundColor() {
  return _model->getBackgroundColor();
}

void DisplayFieldController::setBackgroundColor(string colorName) {
  _model->setBackgroundColor(colorName);
}

void DisplayFieldController::setVisible(size_t fieldIndex) {
  _model->setVisible(fieldIndex);
}



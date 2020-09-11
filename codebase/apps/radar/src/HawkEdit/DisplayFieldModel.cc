
#include "toolsa/LogStream.hh"
#include "DisplayFieldModel.hh"
#include "DisplayField.hh"
#include "ColorMap.hh"
#include "ColorBar.hh"
#include "ColorMapTemplates.hh"


DisplayFieldModel::DisplayFieldModel(vector<DisplayField *> displayFields,
				     string selectedFieldName,
				     string gridColor,
				     string emphasisColor,
				     string annotationColor,
				     string backgroundColor)
{
  LOG(DEBUG) << "enter";
  _fields = displayFields;

  LOG(DEBUG) << "Field names ... ";
  for (vector<DisplayField *>::iterator fieldItr = _fields.begin(); fieldItr != _fields.end(); fieldItr++) {
    DisplayField *field = *fieldItr;
    LOG(DEBUG) << field->getName();
  }
  
  LOG(DEBUG) << "selected field is " << selectedFieldName;
  _selectedFieldName = selectedFieldName;
  _selectedFieldIndex = _lookupFieldIndex(selectedFieldName); 
  _gridColor = gridColor;
  _emphasisColor = emphasisColor;
  _annotationColor = annotationColor;
  _backgroundColor = backgroundColor;
  LOG(DEBUG) << "exit";
}

DisplayFieldModel::~DisplayFieldModel() {

  // free  _workingCopies;
}

void DisplayFieldModel::addField(DisplayField *newField) {
  _fields.push_back(newField);
}

vector<string>  DisplayFieldModel::getFieldNames() {
  vector<string> fieldNames;
  for (vector<DisplayField *>::iterator fieldItr = _fields.begin(); fieldItr != _fields.end(); fieldItr++) {
    DisplayField *field = *fieldItr;
    fieldNames.push_back(field->getName());
  }
 
  return fieldNames;

} 

size_t DisplayFieldModel::getNFields() {
  return _fields.size();
}

DisplayField *DisplayFieldModel::getField(size_t fieldIdx) {
  return _fields[fieldIdx];
}

DisplayField *DisplayFieldModel::getField(string fieldName) {
  return _findFieldByName(fieldName);
}

size_t DisplayFieldModel::getFieldIndex(string fieldName) {
  return _lookupFieldIndex(fieldName);
}

string DisplayFieldModel::getFieldName(size_t fieldIndex) {
  DisplayField *displayField = getField(fieldIndex);
  if (displayField == NULL) {
    LOG(DEBUG) << "field name not found for index " << fieldIndex;
    throw std::invalid_argument("field index not found");
  }
  return displayField->getName();
}

string DisplayFieldModel::getFieldAlias(string fieldName) {
  DisplayField *displayField = getField(fieldName);
  if (displayField == NULL) 
    throw std::invalid_argument(fieldName);
  return displayField->getLabel();
}

string DisplayFieldModel::getSelectedFieldName() {
  return _selectedFieldName;
} 

DisplayField *DisplayFieldModel::getSelectedField() {
  return _findFieldByName(_selectedFieldName);
} 

void DisplayFieldModel::setSelectedField(string fieldName) {
  LOG(DEBUG) << "enter " << fieldName;
  _selectedFieldName = fieldName;
  _selectedFieldIndex = _lookupFieldIndex(fieldName);
  LOG(DEBUG) << "exit";
} 

void DisplayFieldModel::setSelectedField(size_t fieldIndex) {
  if ((fieldIndex >= 0) && (fieldIndex < _fields.size())) {
    _selectedFieldIndex = fieldIndex;
    DisplayField *displayField = getField(fieldIndex);
    _selectedFieldName = displayField->getName();
  }
  else
    throw std::invalid_argument("field index out of bounds");
}

/*
bool DisplayFieldModel::getChanges() {

   //show();
   return false;

}
*/

DisplayField *DisplayFieldModel::getFiltered(size_t ifield, int buttonRow)
{
  DisplayField *filtField = NULL;
  if (ifield < _fields.size() - 1) {
    //if (_fields[ifield+1]->getButtonRow() == buttonRow &&
    if (_fields[ifield+1]->getIsFilt()) {
      filtField = _fields[ifield+1];
    }
  }
  return filtField;
}

ColorMap *DisplayFieldModel::_getOriginalColorMap(string fieldName) {

  LOG(DEBUG) << "looking for field ..." <<  fieldName;

  // find the field or return NULL if not found
  ColorMap *colorMap = NULL;

  bool found = false;
  vector<DisplayField *>::iterator it;
  it = _fields.begin(); 
  while (it != _fields.end() && !found) {
    DisplayField *field = *it;

    string name = field->getName();
    LOG(DEBUG) << "comparing to ..." << name;
    if (name.compare(fieldName) == 0) {
      LOG(DEBUG) << "found color map";
      found = true;
      const ColorMap &colorMapOriginal = field->getColorMap();
      colorMap = new ColorMap(colorMapOriginal);
    }
    it++;
  }
  if (!found) {
    LOG(ERROR) << "ERROR - field not found" << fieldName;
  } 
  return colorMap;
}

// make a working copy of the colorMaps ... 
// return NULL if not found
ColorMap *DisplayFieldModel::getColorMap(string fieldName) {
  
  LOG(DEBUG) << "entry " << fieldName;

  ColorMap *workingCopyColorMap = NULL;

  // first, look in the working copies
  map<string, ColorMap *>::iterator it = _workingCopies.find(fieldName);
  if (it != _workingCopies.end()) {
    LOG(DEBUG) << "found in the workingCopies";
    workingCopyColorMap = it->second; 
  } else {
    LOG(DEBUG) << " need to make a copy";
    // if no working copy, then make one and insert it into list
    //if (!found) {
    workingCopyColorMap = _getOriginalColorMap(fieldName);
    if (workingCopyColorMap != NULL) {
      _workingCopies[fieldName] = workingCopyColorMap;
    }
  }
  // setSelectedField(fieldName);

  if (0) workingCopyColorMap->print(cout);

  LOG(DEBUG) << "exit";
  
  return workingCopyColorMap;
}

// make a working copy of the colorMaps ... 
// return NULL if not found
void DisplayFieldModel::setColorMap(string fieldName, ColorMap *newColorMap) {
  
  LOG(DEBUG) << "entry " << fieldName;

  // first, look in the working copies
  map<string, ColorMap *>::iterator it = _workingCopies.find(fieldName);
  if (it != _workingCopies.end()) {
    LOG(DEBUG) << "found in the workingCopies";
    _workingCopies.erase(it);  
  } 

  // insert new version into list
  _workingCopies[fieldName] = newColorMap;

  LOG(DEBUG) << "exit";
  
}

// save/perpetuate the color map to the DisplayField object ... 
// return NULL if not found
void DisplayFieldModel::saveColorMap(string fieldName, ColorMap *newColorMap) {
  
  LOG(DEBUG) << "entry " << fieldName;

  DisplayField *displayField = _findFieldByName(fieldName);
  displayField->replaceColorMap(*newColorMap);

  /*
  // first, look in the working copies
  map<string, ColorMap *>::iterator it = _workingCopies.find(fieldName);
  if (it != _workingCopies.end()) {
    LOG(DEBUG) << "found in the workingCopies";
    _workingCopies.erase(it);  
  } 

  // insert new version into list
  _workingCopies[fieldName] = newColorMap;
  */


  LOG(DEBUG) << "exit";
  
}

void DisplayFieldModel::setColorMapMinMax(string fieldName, double min, double max) {
  DisplayField *field = _findFieldByName(fieldName);
  field->setColorMapRange(min, max);
  field->changeColorMap();
}


ColorMap *DisplayFieldModel::colorMapMaxChanged(string fieldName, double newValue) {
  LOG(DEBUG) << "entry " << newValue;
  LOG(DEBUG) << "affected fieldName " << fieldName;

  ColorMap *workingVersion = getColorMap(fieldName);

  if (newValue != workingVersion->rangeMax()) {
    workingVersion->setRangeMax(newValue);
  }
  LOG(DEBUG) << "colorMap after max changed";
  workingVersion->print(cout);
  LOG(DEBUG) << "exit";
  return workingVersion;
}

ColorMap *DisplayFieldModel::colorMapMinChanged(string fieldName, double newValue) {
  LOG(DEBUG) << "entry " << newValue;
  LOG(DEBUG) << "affected fieldName " << fieldName;

  ColorMap *workingVersion = getColorMap(fieldName);

  if (newValue != workingVersion->rangeMin()) {
    workingVersion->setRangeMin(newValue);
  }
  LOG(DEBUG) << "colorMap after min changed";
  workingVersion->print(cout);
  LOG(DEBUG) << "exit";
  return workingVersion;
}

ColorMap *DisplayFieldModel::colorMapMaxChanged(double newValue) {
  LOG(DEBUG) << "entry " << newValue;
  LOG(DEBUG) << "_selectedFieldName " << _selectedFieldName;

  ColorMap *workingVersion = getColorMap(_selectedFieldName);

  if (newValue != workingVersion->rangeMax()) {
    /* create new ColorMap because just setting the range doesn't work
    string currentColorMapName = workingVersion->getName();
    LOG(DEBUG) << "current ColorMap name " << currentColorMapName;
    ColorMap *newColorMap = new ColorMap(workingVersion->rangeMin(), newValue,
           currentColorMapName);
    delete workingVersion;
    workingVersion = newColorMap;
    _workingCopies[_selectedFieldName] = newColorMap;
    */
    workingVersion->setRangeMax(newValue);
  }
  LOG(DEBUG) << "colorMap after max changed";
  workingVersion->print(cout);
  LOG(DEBUG) << "exit";
  return workingVersion;
}

ColorMap *DisplayFieldModel::colorMapMinChanged(double newValue) {
  LOG(DEBUG) << "entry " << newValue;
  LOG(DEBUG) << "_selectedFieldName " << _selectedFieldName;

  ColorMap *workingVersion = getColorMap(_selectedFieldName);

  if (newValue != workingVersion->rangeMin()) {
    workingVersion->setRangeMin(newValue);
  }
  LOG(DEBUG) << "colorMap after min changed";
  workingVersion->print(cout);
  LOG(DEBUG) << "exit";
  return workingVersion;
}

void DisplayFieldModel::colorMapChanged(string newColorMapName) {
  LOG(DEBUG) << "enter";
  // change the ColorMap for the currently selected field
  ColorMap *workingVersion = getColorMap(_selectedFieldName);

  // maintain the current min, max, step, center points
  ColorMap *colorMap;
  colorMap = new ColorMap(workingVersion->rangeMin(), 
				       workingVersion->rangeMax(), newColorMapName);
  //  newColorMap->setRangeMax(workingVersion->rangeMax());  
  //newColorMap->setRangeMin(workingVersion->rangeMin());  
  // currently only using built in names
  setColorMap(_selectedFieldName, colorMap);

  
  LOG(DEBUG) << "exit";
}

void DisplayFieldModel::colorMapChanged(string fieldName, string newColorMapName) {
  LOG(DEBUG) << "enter";
  // change the ColorMap for the fieldName
  ColorMap *workingVersion = getColorMap(fieldName);

  // maintain the current min, max, step, center points
  ColorMap *colorMap;
  // look for the colorMap in the import list first.
  // if not found, look in the built in list. 
  // ColorMap returns a default color map if not found in the built in list.
  ColorMapTemplates *colorMapTemplates;
  colorMapTemplates = colorMapTemplates->getInstance();
  colorMap = colorMapTemplates->getColorMap(newColorMapName);
  if (colorMap == NULL) {
    colorMap = new ColorMap(workingVersion->rangeMin(), 
               workingVersion->rangeMax(), newColorMapName);
  }
  //  newColorMap->setRangeMax(workingVersion->rangeMax());  
  //newColorMap->setRangeMin(workingVersion->rangeMin());  
  // currently only using built in names
  setColorMap(fieldName, colorMap);

  
  LOG(DEBUG) << "exit";
}

//void DisplayFieldModel::colorMapChanged(string fieldName, ColorMap *colorMap) {
//  LOG(DEBUG) << "enter";
  // change the ColorMap for the fieldName
  //ColorMap *workingVersion = getColorMap(fieldName);

  // maintain the current min, max, step, center points
  //ColorMap *colorMap;
  //colorMap = new ColorMap(workingVersion->rangeMin(), 
 //              workingVersion->rangeMax(), newColorMapName);
  //  newColorMap->setRangeMax(workingVersion->rangeMax());  
  //newColorMap->setRangeMin(workingVersion->rangeMin());  
  // currently only using built in names
 // setColorMap(fieldName, colorMap);
  
//  LOG(DEBUG) << "exit";
//}

bool DisplayFieldModel::backgroundChanged(string fieldName) {
  LOG(DEBUG) << fieldName;
  LOG(DEBUG) << "background changed";
  return false;
}

string DisplayFieldModel::getGridColor() {
  return _gridColor;
}

void DisplayFieldModel::setGridColor(string colorName) {
  LOG(DEBUG) << "enter " << colorName;
  _gridColor = colorName;
  LOG(DEBUG) << "exit";
}

string DisplayFieldModel::getEmphasisColor() {
  return _emphasisColor;
}

void DisplayFieldModel::setEmphasisColor(string colorName) {
  _emphasisColor = colorName;
}

string DisplayFieldModel::getAnnotationColor() {
  return _annotationColor;
}

void DisplayFieldModel::setAnnotationColor(string colorName) {
  _annotationColor = colorName;
}

string DisplayFieldModel::getBackgroundColor() {
  return _backgroundColor;
}

void DisplayFieldModel::setBackgroundColor(string colorName) {
  _backgroundColor = colorName;
}

void DisplayFieldModel::setForLocationClicked(double value, const string &text) {
  for (size_t ii = 0; ii < _fields.size(); ii++) {
    _fields[ii]->setSelectValue(value);
    _fields[ii]->setDialogText(text);
  }
}

void DisplayFieldModel::setForLocationClicked(string fieldName, double value, const string \
						   &text) {
  // TODO: use _findFieldByName
  for (size_t ii = 0; ii < _fields.size(); ii++) {
    if (fieldName == _fields[ii]->getName()) {
      _fields[ii]->setSelectValue(value);
      _fields[ii]->setDialogText(text);
    }
  }
}

DisplayField *DisplayFieldModel::_findFieldByName(string fieldName) {
  DisplayField *theField = NULL;
  for (size_t ii = 0; ii < _fields.size(); ii++) {
    if (fieldName == _fields[ii]->getName()) {
      theField = _fields[ii];
    }
  }
  return theField;
}

size_t DisplayFieldModel::_lookupFieldIndex(string fieldName) {
  for (size_t ii = 0; ii < _fields.size(); ii++) {
    if (fieldName == _fields[ii]->getName()) {
      return ii;
    }
  }
  throw std::invalid_argument("field name not found");
}

void DisplayFieldModel::setVisible(size_t fieldIndex) {
  if (fieldIndex < _fields.size())
    _fields.at(fieldIndex)->setStateVisible();
  else 
    LOG(ERROR) << "fieldIndex out of bounds " << fieldIndex;
}


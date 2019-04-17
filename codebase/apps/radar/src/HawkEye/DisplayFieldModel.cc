
#include "toolsa/LogStream.hh"
#include "DisplayFieldModel.hh"
#include "DisplayField.hh"
#include "ColorMap.hh"
#include "ColorBar.hh"


DisplayFieldModel::DisplayFieldModel(vector<DisplayField *> displayFields, string selectedFieldName)
{
  LOG(DEBUG) << "enter";
  _fields = displayFields;
  _selectedFieldName = selectedFieldName;
  LOG(DEBUG) << "exit";
}

DisplayFieldModel::~DisplayFieldModel() {

  // free  _workingCopies;
}


vector<string>  DisplayFieldModel::getFieldNames() {
  vector<string> fieldNames;
  for (vector<DisplayField *>::iterator fieldItr = _fields.begin(); fieldItr != _fields.end(); fieldItr++) {
    DisplayField *field = *fieldItr;
    fieldNames.push_back(field->getName());
  }
 
  return fieldNames;

} 


string DisplayFieldModel::getSelectedField() {

  return _selectedFieldName;
} 

void DisplayFieldModel::setSelectedField(string fieldName) {
  LOG(DEBUG) << "enter " << fieldName;
  _selectedFieldName = fieldName;
  LOG(DEBUG) << "exit";
} 

/*
bool DisplayFieldModel::getChanges() {

   //show();
   return false;

}
*/

ColorMap *DisplayFieldModel::_getOriginalColorMap(string fieldName) {

  LOG(DEBUG) << "looking for field ...";
  LOG(DEBUG) << fieldName;

  // find the field or return NULL if not found
  ColorMap *colorMap = NULL;

  bool found = false;
  vector<DisplayField *>::iterator it;
  it = _fields.begin(); 
  while (it != _fields.end() && !found) {
    DisplayField *field = *it;

    string name = field->getName();
    LOG(DEBUG) << "comparing to ...";
    LOG(DEBUG) << name;
    if (name.compare(fieldName) == 0) {
      LOG(DEBUG) << "found color map";
      found = true;
      const ColorMap &colorMapOriginal = field->getColorMap();
      colorMap = new ColorMap(colorMapOriginal);
    }
    it++;
  }
  if (!found) {
    LOG(ERROR) << fieldName;
    LOG(ERROR) << "ERROR - field not found";                                                           
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
  setSelectedField(fieldName);

  workingCopyColorMap->print(cout);

  LOG(DEBUG) << "exit";
  
  return workingCopyColorMap;
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

bool DisplayFieldModel::colorMapChanged(string fieldName) {
  LOG(DEBUG) << fieldName;
  LOG(DEBUG) << "color map changed";
  return false;
}

bool DisplayFieldModel::backgroundChanged(string fieldName) {
  LOG(DEBUG) << fieldName;
  LOG(DEBUG) << "background changed";
  return false;
}

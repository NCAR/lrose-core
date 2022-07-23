
#include "toolsa/LogStream.hh"
#include "DisplayFieldModel.hh"
#include "DisplayField.hh"
#include <qtplot/ColorMap.hh>
#include "ColorBar.hh"


DisplayFieldModel::DisplayFieldModel(vector<DisplayField *> displayFields,
				     string selectedFieldName,
				     string gridColor,
				     string emphasisColor,
				     string annotationColor,
				     string backgroundColor)
{
  LOG(DEBUG) << "enter";
  _fields = displayFields;
  _selectedFieldName = selectedFieldName;
  _gridColor = gridColor;
  _emphasisColor = emphasisColor;
  _annotationColor = annotationColor;
  _backgroundColor = backgroundColor;
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



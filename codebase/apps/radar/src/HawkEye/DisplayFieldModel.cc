
#include "toolsa/Logstream.hh"
#include "DisplayFieldModel.hh"
#include "DisplayField.hh"
#include "ColorMap.hh"
#include "ColorBar.hh"


DisplayFieldModel::DisplayFieldModel(vector<DisplayField *> displayFields, string selectedFieldName)
{
  _fields = displayFields;
  _selectedFieldName = selectedFieldName;
}

DisplayFieldModel::~DisplayFieldModel() {}


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

bool DisplayFieldModel::getChanges() {

   //show();
   return false;

}

ColorMap *DisplayFieldModel::getColorMap(string fieldName) {

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


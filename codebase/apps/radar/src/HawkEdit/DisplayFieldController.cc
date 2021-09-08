
#include <toolsa/LogStream.hh>
#include <toolsa/Path.hh>
#include "DisplayFieldController.hh"
#include "SoloDefaultColorWrapper.hh"
//#include "DisplayField.hh"
#include "ColorMap.hh"
#include "ColorBar.hh"
#include "Params.hh"


DisplayFieldController::DisplayFieldController(DisplayFieldModel *model)
{
  LOG(DEBUG) << "enter";
  _model = model;
  LOG(DEBUG) << "exit";
}

DisplayFieldController::~DisplayFieldController() {
  //  delete _model;
}

// not used??
void clearAllFields() {
  //_view->clearAllFields();  ->> or just delete fields that are not needed???
  //_model->clearAllFields(); 
}

bool DisplayFieldController::contains(string fieldName) {
  bool present = false;
  try {
    if (_model->getFieldIndex(fieldName) >= 0) {
      present = true;
    }
  } catch (std::invalid_argument &ex) {
      LOG(DEBUG) << fieldName << " not present";
  }
  return present;
}

void DisplayFieldController::addField(DisplayField *newField) {
  string fieldName = newField->getName();

  DisplayField *displayField = _model->getField(fieldName);
  if (displayField == NULL) { // field is not already in list
    if (!_endWithV(fieldName)) {
      _model->addField(newField);
    }
  } else {
    LOG(DEBUG) << "field already in list: " << fieldName; 
  }

}

bool DisplayFieldController::_endWithV(string &fieldName) {
  size_t length = fieldName.length();
  if (length < 2) return false;
  if ((fieldName[length-1] == 'V') && (fieldName[length-2] == '_')) {
    return true;
  } else {
    return false;
  }
}

void DisplayFieldController::addField(string &fieldName) {
  // ignore the _V fields
  if (!_endWithV(fieldName)) {
    DisplayField *displayField = _model->getField(fieldName);
    if (displayField == NULL) {
      DisplayField *newDisplayField = new DisplayField(fieldName);
      addField(newDisplayField);
      // this is done by the PolarManager ...
      //_displayFieldView->updateFieldPanel(newFieldName, newFieldName, newFieldName);
    } else {
      //updateFieldPanel(newFieldName);
    }
  }
}


void DisplayFieldController::updateFieldPanel(string fieldName,
  DisplayFieldView *fieldPanel) {
  //_displayFieldView->updateFieldPanel(fieldName);

  LOG(DEBUG) << "enter";
  if (!contains(fieldName)) {
    addField(fieldName);
    LOG(DEBUG) << "adding fieldName " << fieldName;
  }
  size_t index = getFieldIndex(fieldName);
  DisplayField *rawField = getField(index);
  if (rawField->isHidden()) { 
    rawField->setStateVisible();
    fieldPanel->updateFieldPanel(fieldName, fieldName,
      fieldName);
  }
  setSelectedField(index);
  LOG(DEBUG) << "exit";
}

void DisplayFieldController::reconcileFields(vector<string> *fieldNames,
  DisplayFieldView *fieldPanel) {

  fieldPanel->clear();

    // remove current fields that are no longer needed
  vector<string> currentFieldsNames = getFieldNames();

  for (vector<string>::iterator currentName = currentFieldsNames.begin(); 
    currentName != currentFieldsNames.end(); ++currentName) {
  
    std::vector<string>::iterator it;

    it = std::find(fieldNames->begin(), fieldNames->end(), *currentName);
    if (it != fieldNames->end()) {
      LOG(DEBUG) << "displayField found in list of new fields: " << *it << " keep it";
    }
    else {
      LOG(DEBUG) << "displayField not found in list of new fields" << *currentName << " discarding";
      deleteField(*currentName);
      // TODO how is the field removed from the panel?
      //fieldPanel->delete(*currentName);
    }
  }


  //for (int ifield = 0; ifield < _params->fields_n; ifield++) {
  int ifield = (int) getNFields() + 1;
  for (vector<string>::iterator it = fieldNames->begin(); it != fieldNames->end(); ++it) {
    string fieldName = *it;

//HERE TODO:
//distingquish between add and update on fieldName;
//then set last field or first field as selected? or do something to render image

    DisplayField *displayField;
    if (!contains(fieldName)) {

      ColorMap map;
      map = ColorMap(0.0, 1.0);
      bool noColorMap = true; 
      // unfiltered field
      string shortcut = to_string(ifield);
      displayField = new DisplayField(fieldName, fieldName, "m/s", 
                         shortcut, map, ifield, false);
      if (noColorMap)
        displayField->setNoColorMap();

      addField(displayField);

      //_updateFieldPanel(fieldName);
      // TODO: causes a EXC_BAD_ACCESS if outside the loop
      // somehow this is called when setting up the menus???
      //fieldPanel->updateFieldPanel(fieldName, fieldName, fieldName);
      ifield += 1;
    } else {
      size_t index = getFieldIndex(fieldName);
      displayField = getField(index);
    } 
    displayField->setStateVisible();
    fieldPanel->updateFieldPanel(fieldName, fieldName, fieldName);

  } // ifield

}

void DisplayFieldController::deleteField(string &fieldName) {
  _model->deleteField(fieldName);
}

void DisplayFieldController::dataFileChanged() {

  // reconcile the data fields with those already in the panel

}

void DisplayFieldController::fieldSelected(string fieldName) {

}

void DisplayFieldController::hideField(DisplayField *field) {
  //_model->hideField(field);
}

void DisplayFieldController::setFieldToMissing(const string &fieldName) {
  //_model->setFieldToMissing(fieldName);
}

// Remove field from data volume
void DisplayFieldController::deleteFieldFromVolume(const string &fieldName) {
  //_model->deleteFieldFromVolume(fieldName);
}




//void DisplayFieldController::delete(string fieldName) {
//  _model->delete(fieldName);
//}

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
  //notifyFieldChange();
  LOG(DEBUG) << "exit";
} 


void DisplayFieldController::setSelectedField(size_t fieldIndex) {
  LOG(DEBUG) << "enter";
  _model->setSelectedField(fieldIndex);
  //notifyFieldChange();
  LOG(DEBUG) << "exit";
} 

/*
void DisplayFieldController::notifyFieldChange() {
   // the DisplayFieldController notifies everyone that
//the field has changed?  the order should be:
//DisplayFieldModel is updated
  bool guiMode = true; 
  emit selectedFieldChanged(); 
}
*/

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


//////////////////////////////////////////////////
// set up field objects, with their color maps
// use same map for raw and unfiltered fields
// returns 0 on success, -1 on failure
  // TODO: move to DisplayFieldController
//int HawkEye::_setupDisplayFields()
void DisplayFieldController::setupDisplayFields(
  string colorMapDir, 
  vector<Params::field_t> &fields,
  string gridColor, 
  string emphasisColor,
  string annotationColor, 
  string backgroundColor,
  Params::debug_t debug  
  )
{

  // check for color map location
  //ParamFile *_params = ParamFile::Instance();

  vector<DisplayField *> _displayFields;
  
  //colorMapDir = _params->color_scale_dir;
  Path mapDir(colorMapDir);
  if (!mapDir.dirExists()) {
    colorMapDir = Path::getPathRelToExec(colorMapDir);
    mapDir.setPath(colorMapDir);
    if (!mapDir.dirExists()) {
      cerr << "ERROR - HawkEdit" << endl;
      cerr << "  Cannot find color scale directory" << endl;
      cerr << "  Primary is: " << colorMapDir << endl;
      cerr << "  Secondary is relative to binary: " << colorMapDir << endl;
      throw "ERROR - cannot find color scale directory " + colorMapDir;
    }
    if (debug) {
      cerr << "NOTE - using color scales relative to executable location" << endl;
      cerr << "  Exec path: " << Path::getExecPath() << endl;
      cerr << "  Color scale dir:: " << colorMapDir << endl;
    }
  }

  // we interleave unfiltered fields and filtered fields

  for (int ifield = 0; ifield < fields.size(); ifield++) {

    const Params::field_t &pfld = fields.at(ifield);

    // check we have a valid label
    
    if (strlen(pfld.label) == 0) {
      cerr << "WARNING - HawkEye::_setupDisplayFields()" << endl;
      cerr << "  Empty field label, ifield: " << ifield << endl;
      cerr << "  Ignoring" << endl;
      continue;
    }
    
    // check we have a raw field name
    
    if (strlen(pfld.raw_name) == 0) {
      cerr << "WARNING - HawkEye::_setupDisplayFields()" << endl;
      cerr << "  Empty raw field name, ifield: " << ifield << endl;
      cerr << "  Ignoring" << endl;
      continue;
    }

    // create color map
    
    string colorMapPath = colorMapDir;
    colorMapPath += PATH_DELIM;
    colorMapPath += pfld.color_map;
    ColorMap map;
    map.setName(pfld.label);
    map.setUnits(pfld.units);
    // TODO: the logic here is a little weird ... the label and units have been set, but are we throwing them away?

    bool noColorMap = false;

    if (map.readMap(colorMapPath)) {
        cerr << "WARNING - HawkEye::_setupDisplayFields()" << endl;
        cerr << "  Cannot read in color map file: " << colorMapPath << endl;
        cerr << "  Looking for default color map for field " << pfld.label << endl; 

        try {
          // check here for smart color scale; look up by field name/label and
          // see if the name is a usual parameter for a known color map
          SoloDefaultColorWrapper sd = SoloDefaultColorWrapper::getInstance();
          ColorMap colorMap = sd.ColorMapForUsualParm.at(pfld.label);
          cerr << "  found default color map for " <<  pfld.label  << endl;
          // if (_params.debug) colorMap.print(cout); // LOG(DEBUG_VERBOSE)); // cout);
          map = colorMap;
          // HERE: What is missing from the ColorMap object??? 
        } catch (std::out_of_range ex) {
          cerr << "WARNING - did not find default color map for field; using rainbow colors" << endl;
    // Just set the colormap to a generic color map
    // use range to indicate it needs update; update when we have access to the actual data values
          map = ColorMap(0.0, 1.0);
          noColorMap = true; 
        }
    }

    // unfiltered field

    DisplayField *field =
      new DisplayField(pfld.label, pfld.raw_name, pfld.units, 
                       pfld.shortcut, map, ifield, false);
    if (noColorMap)
      field->setNoColorMap();

    _displayFields.push_back(field);

    // filtered field

    if (strlen(pfld.filtered_name) > 0) {
      string filtLabel = string(pfld.label) + "-filt";
      DisplayField *filt =
        new DisplayField(filtLabel, pfld.filtered_name, pfld.units, pfld.shortcut, 
                         map, ifield, true);
      _displayFields.push_back(filt);
    }

  } // ifield

  if (_displayFields.size() < 1) {
    cerr << "ERROR - HawkEdit::_setupDisplayFields()" << endl;
    cerr << "  No fields found" << endl;
    throw "ERROR - HawkEdit::_setupDisplayFields no fields found";
  } else {
    delete _model;
    string selectedFieldName = _displayFields.at(0)->getName();
    _model = new  DisplayFieldModel(_displayFields, selectedFieldName,
        gridColor, emphasisColor,
        annotationColor, backgroundColor);
  }

}

/*
void DisplayFieldController::setView(DisplayFieldView *view) {
 _displayFieldView = view;
//connect(_displayFieldView, SIGNAL(changeToField(string)),
//        this, SLOT(setSelectedField(string)));
}

//void DisplayFieldController::createFieldPanel(QFrame *mainFrame) {
//  _displayFieldView->createFieldPanel(mainFrame);
//}
*/

// TODO: make this a signal and slot??
//QImage &DisplayFieldController::getSelectedFieldImage() {

  //DisplayField *selectedField = _model->getSelectedField();
  //QImage *FieldRendererController::renderImage(int width, int height,
  //string fieldName, double sweepAngle, 
  //RayLocationController *rayLocationController,
  //ColorMap &colorMap,
  //QColor backgroundColor)
  //return _fieldRendererController->renderImage(fieldName ...);
  //return selectedField->getImage();
//};

/*
void DisplayFieldController::renderFields() {
  LOG(DEBUG) << "enter";
  DisplayField *selectedField = getSelectedField();
  selectedField->render(100,100);
  LOG(DEBUG) << "exit";
}
*/

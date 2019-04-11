#ifndef DISPLAYFIELDMODEL_H
#define DISPLAYFIELDMODEL_H

#include "DisplayFieldModel.hh"
#include "DisplayManager.hh"
#include "ColorMap.hh"
//#include "../HawkEye/ColorBar.hh"


class DisplayFieldModel
{

public:

  DisplayFieldModel(vector<DisplayField *> displayFields, string selectedFieldName);
  ~DisplayFieldModel();

  vector<string> getFieldNames();
  string getSelectedField();
  bool getChanges();
  ColorMap *getColorMap(string fieldName);
  bool colorMapChanged(string fieldName);
  bool backgroundChanged(string fieldName);

private:

  vector<DisplayField *> _fields;
  string _selectedFieldName;
};

#endif

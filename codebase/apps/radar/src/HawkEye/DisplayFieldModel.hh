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
  void setSelectedField(string fieldName);
  bool getChanges();
  ColorMap *getColorMap(string fieldName);
  bool colorMapChanged(string fieldName);
  bool backgroundChanged(string fieldName);

  ColorMap *colorMapMaxChanged(double newValue);
  ColorMap *colorMapMinChanged(double newValue);


private:

  ColorMap *_getOriginalColorMap(string fieldName);

  vector<DisplayField *> _fields;
  string _selectedFieldName;
  map<string, ColorMap *> _workingCopies;
};

#endif

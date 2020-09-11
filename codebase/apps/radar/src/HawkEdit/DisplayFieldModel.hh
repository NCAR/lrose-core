#ifndef DISPLAYFIELDMODEL_H
#define DISPLAYFIELDMODEL_H

#include "DisplayFieldModel.hh"
#include "DisplayField.hh"
#include "ColorMap.hh"


class DisplayFieldModel
{

public:

  DisplayFieldModel(vector<DisplayField *> displayFields, string selectedFieldName,
		    string gridColor, string emphasisColor,
		    string annotationColor, string backgroundColor);
  ~DisplayFieldModel();

  void addField(DisplayField *newField);

  vector<string> getFieldNames();
  size_t getNFields();

  DisplayField *getField(size_t fieldIdx);
  DisplayField *getField(string fieldName);
  size_t getFieldIndex(string fieldName);
  string getFieldName(size_t fieldIndex);

  string getFieldAlias(string fieldName);

  string getSelectedFieldName();
  DisplayField *getSelectedField();
  size_t getSelectedFieldNum() { return _selectedFieldIndex;};

  void setSelectedField(string fieldName);
  void setSelectedField(size_t fieldIndex);

  DisplayField *getFiltered(size_t ifield, int buttonRow);

  void setForLocationClicked(double value, const string &text);
  void setForLocationClicked(string fieldName, double value, const string &text);

  //  bool getChanges();
  ColorMap *getColorMap(string fieldName);
  ColorMap *getColorMap(size_t fieldIndex);
  void setColorMap(string fieldName, ColorMap *newColorMap);
  void saveColorMap(string fieldName, ColorMap *newColorMap);
  void setColorMapMinMax(string fieldName, double min, double max);
  void colorMapChanged(string newColorMapName);
  void colorMapChanged(string fieldName, string newColorMapName);
  bool backgroundChanged(string fieldName);

  //  void colorMapMaxChanged(ColorMap *newColorMap);
  ColorMap *colorMapMaxChanged(double newValue);
  ColorMap *colorMapMinChanged(double newValue);

  ColorMap *colorMapMaxChanged(string fieldName, double newValue);
  ColorMap *colorMapMinChanged(string fieldName, double newValue);

  string getGridColor();
  void setGridColor(string colorName);

  string getEmphasisColor();
  void setEmphasisColor(string colorName);

  string getAnnotationColor();
  void setAnnotationColor(string colorName);

  string getBackgroundColor();
  void setBackgroundColor(string colorName);

  void setVisible(size_t fieldIndex);

private:

  ColorMap *_getOriginalColorMap(string fieldName);
  DisplayField *_findFieldByName(string fieldName);
  size_t _lookupFieldIndex(string fieldName);

  vector<DisplayField *> _fields;
  // TODO: only keep one of these ... 
  string _selectedFieldName;
  size_t _selectedFieldIndex;
  // these define the current state of editing ...
  // each time there is a replot, save the state?

  // the min, max, center, step are all kept in the ColorMap

  map<string, ColorMap *> _workingCopies;
  // the colors are stored as strings as blue, white, etc,
  // or as #RRGGBB
  string _gridColor;
  string _emphasisColor;
  string _annotationColor;
  string _backgroundColor;
};

#endif

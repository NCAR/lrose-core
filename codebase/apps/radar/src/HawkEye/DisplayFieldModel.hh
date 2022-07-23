#ifndef DISPLAYFIELDMODEL_H
#define DISPLAYFIELDMODEL_H

#include "DisplayFieldModel.hh"
#include "DisplayManager.hh"
#include <qtplot/ColorMap.hh>
//#include "../HawkEye/ColorBar.hh"


class DisplayFieldModel
{

public:

  DisplayFieldModel(vector<DisplayField *> displayFields, string selectedFieldName,
		    string gridColor, string emphasisColor,
		    string annotationColor, string backgroundColor);
  virtual ~DisplayFieldModel();

  vector<string> getFieldNames();
  string getSelectedField();
  void setSelectedField(string fieldName);
  //  bool getChanges();
  ColorMap *getColorMap(string fieldName);
  void setColorMap(string fieldName, ColorMap *newColorMap);
  void colorMapChanged(string newColorMapName);
  bool backgroundChanged(string fieldName);

  //  void colorMapMaxChanged(ColorMap *newColorMap);
  ColorMap *colorMapMaxChanged(double newValue);
  ColorMap *colorMapMinChanged(double newValue);

  string getGridColor();
  void setGridColor(string colorName);

  string getEmphasisColor();
  void setEmphasisColor(string colorName);

  string getAnnotationColor();
  void setAnnotationColor(string colorName);

  string getBackgroundColor();
  void setBackgroundColor(string colorName);

private:

  ColorMap *_getOriginalColorMap(string fieldName);

  vector<DisplayField *> _fields;
  string _selectedFieldName;

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

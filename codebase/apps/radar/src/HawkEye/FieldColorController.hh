#ifndef FIELDCOLORCONTROLLER_H
#define FIELDCOLORCONTROLLER_H


#include <QObject>

#include "ParameterColorView.hh"
#include "DisplayFieldModel.hh"
//#include "../HawkEye/ColorMap.hh"
//#include "../HawkEye/ColorBar.hh"


class FieldColorController : public QObject
{
  Q_OBJECT

public:

  FieldColorController(ParameterColorView *parameterColorView,
		       DisplayFieldModel *displayFieldModel);
  ~FieldColorController();

  DisplayFieldModel *_model;
  ParameterColorView *_view;

  //  void getFieldNames();

  void startUp();

signals:
  void colorMapRedefineSent(string fieldName, ColorMap newColorMap);

public slots:
  void getColorMap(string fieldName);
  void colorMapMaxChanged(double newValue);
  void colorMapMinChanged(double newValue);
  void modelChanged(); // string fieldName); // , ColorMap newColorMap);
  void pickColorPaletteRequest();
  void newColorPaletteSelected(string newColorMapName);

private slots:


private:
  ColorMapTemplates *_colorMapTemplates;

};

#endif

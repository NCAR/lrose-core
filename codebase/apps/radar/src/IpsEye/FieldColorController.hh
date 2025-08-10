#ifndef FIELDCOLORCONTROLLER_H
#define FIELDCOLORCONTROLLER_H


#include <QObject>

#include "ParameterColorView.hh"
#include "DisplayFieldModel.hh"
//#include "../IpsEye/ColorMap.hh"
//#include "../IpsEye/ColorBar.hh"


class FieldColorController : public QObject
{
  Q_OBJECT

public:

  FieldColorController(ParameterColorView *parameterColorView,
		       DisplayFieldModel *displayFieldModel);
  virtual ~FieldColorController();

  DisplayFieldModel *_model;
  ParameterColorView *_view;

  //  void getFieldNames();

  void startUp();

signals:
  void colorMapRedefineSent(string fieldName, ColorMap newColorMap,
			    QColor gridColor, 
			    QColor emphasisColor, 
			    QColor annotationColor, 
			    QColor backgroundColor);
  void gridColorSet(QColor newColor);
  void emphasisColorSet(QColor newColor);
  void annotationColorSet(QColor newColor);
  void backgroundColorSet(QColor newColor);

public slots:
  void getColorMap(string fieldName);
  void getGridColor();
  void getEmphasisColor();
  void getAnnotationColor();
  void getBackgroundColor();
  void colorMapMaxChanged(double newValue);
  void colorMapMinChanged(double newValue);
  void modelChanged(); // string fieldName); // , ColorMap newColorMap);
  void pickColorPaletteRequest();
  void newColorPaletteSelected(string newColorMapName);
  void newGridColorSelected(QColor newColor);
  void newEmphasisColorSelected(QColor newColor);
  void newAnnotationColorSelected(QColor newColor);
  void newBackgroundColorSelected(QColor newColor);

private slots:


private:
  ColorMapTemplates *_colorMapTemplates;
  QColor _stringToQColor(string colorName);

  // check that color name is valid
  bool _isValidColorName(QString colorName);

};

#endif

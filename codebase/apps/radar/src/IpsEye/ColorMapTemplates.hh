#ifndef COLORMAPTEMPLATES_H
#define COLORMAPTEMPLATES_H

#include <QDialog>
#include <QLabel>

// #include "FlowLayout.hh"
// #include "Dialog.hh"
//#include "ParameterColorDialog.hh"
#include "ClickableLabel.hh"
#include "ColorMap.hh"
// #include "../IpsEye/ColorBar.hh"


class ColorMapTemplates : public QDialog
{
    Q_OBJECT

public:
    ColorMapTemplates(QWidget *parent = 0);
    virtual ~ColorMapTemplates();

signals:
  void newColorPaletteSelected(string newColorMapName);


private slots:


  void defaultClicked();
  void rainbowClicked();
  void eldoraDbzClicked();
  void eldoraVelClicked();
  void spolVelClicked();
  void spolDivClicked();
  void spolDbzClicked();

private:

  ClickableLabel *_defaultColorMapLabel;
  ClickableLabel *_rainbowColorMapLabel;
  ClickableLabel *_eldoraDbzColorMapLabel;
  ClickableLabel *_spolDbzColorMapLabel;
  ClickableLabel *_eldoraVelColorMapLabel;
  ClickableLabel *_spolVelColorMapLabel;
  ClickableLabel *_spolDivColorMapLabel;


  ColorMap *_defaultColorMap;
  ColorMap *_rainbowColorMap;
  ColorMap *_eldoraDbzColorMap;
  ColorMap *_spolDbzColorMap;
  ColorMap *_eldoraVelColorMap;
  ColorMap *_spolVelColorMap;
  ColorMap *_spolDivColorMap;

};

#endif

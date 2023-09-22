#ifndef DISPLAYFIELDCONTROLLER_H
#define DISPLAYFIELDCONTROLLER_H

#include "DisplayFieldModel.hh"
#include "DisplayFieldView.hh"
//#include "ColorMap.hh"
#include "ParamFile.hh"
#include <QFrame>

class DisplayFieldController
{

public:

  DisplayFieldController(DisplayFieldModel *model);
  virtual ~DisplayFieldController();

  void setupDisplayFields(
    string colorMapDir, 
    vector<Params::field_t> &fields,
    string gridColor, 
    string emphasisColor,
    string annotationColor, 
    string backgroundColor,
    Params::debug_t debug  
  );

  void clearAllFields();

  //void createFieldPanel(QFrame *main);
  void updateFieldPanel(string fieldName, DisplayFieldView *fieldPanel);

  bool contains(string fieldName);
  void addField(DisplayField *newField);
  void addField(string &fieldName);

  void hideField(DisplayField *field);
  void setFieldToMissing(const string &fieldName);
  //void deleteFieldFromVolume(DisplayField *field);
  void deleteFieldFromVolume(const string &fieldName);
  void deleteField(string &fieldName);  
    
  size_t getNFields();

  DisplayField *getField(size_t fieldIndex);
  DisplayField *getField(string fieldName);


  vector<string> getFieldNames();
  string getSelectedFieldName();
  DisplayField *getSelectedField();
  size_t getSelectedFieldNum();

  void setSelectedField(string fieldName);
  void setSelectedField(size_t fieldIndex);
 
  string getFieldAlias(string fieldName);

  DisplayField *getFiltered(size_t fieldIndex, int buttonRow);
  size_t getFieldIndex(string fieldName);


  void setForLocationClicked(double value, const string &text);
  void setForLocationClicked(string fieldName, double value, const string &text);
  //  bool getChanges();
  ColorMap *getColorMap(string fieldName);
  ColorMap *getColorMap(size_t fieldIndex);
  void setColorMap(string fieldName, ColorMap *newColorMap);
  void saveColorMap(string fieldName, ColorMap *newColorMap);
    void colorMapChanged(string newColorMapName);
    bool backgroundChanged(string fieldName);

  void setColorMapMinMax(string fieldName, double min, double max); 
    void colorMapMaxChanged(ColorMap *newColorMap);
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

    void setVisible(size_t fieldIndex);

  DisplayFieldModel *getModel() {return _model;};
  void reconcileFields(vector<string> *fieldNames,
    DisplayFieldView *fieldPanel);



    //void setView(DisplayFieldView *view);

    //void renderFields();
    //QImage &getSelectedFieldImage();

    //void notifyFieldChange();

//signals:

//    void selectedFieldChanged();


//  void setSelectedField(string fieldName);
  void dataFileChanged();
  void fieldSelected(string fieldName);

private:
 
  //vector<DisplayField *> *_current;
  DisplayFieldModel *_model; // edit version 
 
  //DisplayFieldView *_displayFieldView;
  bool _endWithV(string &fieldName); 
  bool _suspendAccess;

};

#endif

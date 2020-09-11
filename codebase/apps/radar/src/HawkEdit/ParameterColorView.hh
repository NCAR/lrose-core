#ifndef PARAMETERCOLORVIEW_H
#define PARAMETERCOLORVIEW_H

#include <QtWidgets>
#include <QPixmap>
#include <QListWidgetItem>
#include "ClickableLabel.hh"

#include "FlowLayout.hh"
//#include "ParameterColorDialog.hh"
#include "ColorMapTemplates.hh"
#include "ColorMap.hh"
#include "ColorBar.hh"

class DialogOptionsWidget;

class ParameterColorView : public QDialog
{
  Q_OBJECT

public:

  ParameterColorView(QWidget *parent = 0); // , ColorMap colorMap);
  ~ParameterColorView();
  void updateEvent(vector<string> fieldNames, string selectedField);
  bool getChanges();
  string getSelectedFieldName() { return _selectedField; };

public slots:
  void colorMapProvided(string fieldName, ColorMap *colorMap);
  void gridColorProvided(QColor color);
  void emphasisColorProvided(QColor color);
  void annotationColorProvided(QColor color);
  void backgroundColorProvided(QColor color);

signals:
  void getColorMap(string selectedField);
  void getGridColor();
  void getEmphasisColor();
  void getAnnotationColor();
  void getBackgroundColor();
  void colorMapMaxChanged(double value);
  void colorMapMinChanged(double value);
  void replotFieldColorMapChanges(); // (string selectedField);
  void pickColorPaletteRequest();
  void gridColorChanged(QColor color);  
  //void backgroundColorChanged(QColor color);  
  //void backgroundColorChanged(QColor color);  
  void backgroundColorChanged(QColor color);  

private slots:

    void setCenterPoint();
    void setMaxPoint();
    void setMinPoint();
    void setStepPoint();
    void setColorOnButton(QPushButton *button, QColor color);
    void setValueOnLineEdit(QLineEdit *editor, double value);
    void setGridColor();
    void setBoundaryColor();
    void setExceededColor();
    void setMissingColor();
    void setAnnotationColor();
    void setBackgroundColor();
    void setEmphasisColor();
    void cancelColorScale();
    void saveColorScale();
    void replotColorScale();
    void pickColorPalette();
  void fieldSelected(QListWidgetItem *current, QListWidgetItem *previous);

private:
    QLabel *centerColorLabel;
    QLineEdit *centerColorLineEdit;
    QLabel *minColorLabel;
    QLineEdit *minColorLineEdit;
    QLabel *maxColorLabel;
    QLineEdit *maxColorLineEdit;
    QLabel *stepColorLabel;
    QLineEdit *stepColorLineEdit;

    QLabel *gridColorLabel;
    QPushButton *gridColorButton;
    QLabel *boundaryColorLabel;
    QPushButton *boundaryColorButton;
    QLabel *exceededColorLabel;
    QPushButton *exceededColorButton;
    QLabel *missingColorLabel;
    QPushButton *missingColorButton;
    QLabel *annotationColorLabel;
    QPushButton *annotationColorButton;
    QLabel *backgroundColorLabel;
    QPushButton *backgroundColorButton;
    QLabel *emphasisColorLabel;
    QPushButton *emphasisColorButton;

    DialogOptionsWidget *colorDialogOptionsWidget;

  ColorMap *currentColorMap;
  ColorBar *colorBar;
  ClickableLabel *cmapLabel;

  QGridLayout *layout;

  QWidget *_parent;

  string _selectedField;

  void errorMessage(string title, string message);
};

#endif

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

  void modelChanged(string fieldName); // , ColorMap newColorMap);

public slots:
  void getColorMap(string fieldName);

private slots:

  /*
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
  */
};

#endif

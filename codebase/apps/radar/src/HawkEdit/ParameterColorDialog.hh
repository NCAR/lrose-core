#ifndef PARAMETERCOLORDIALOG_H
#define PARAMETERCOLORDIALOG_H

#include <QtWidgets>
#include <QPixmap>

#include "FlowLayout.hh"
#include "ParameterColorDialog.hh"
#include "ColorMapTemplates.hh"
//#include "../HawkEye/ColorMap.hh"
//#include "../HawkEye/ColorBar.hh"

class DialogOptionsWidget;

class ParameterColorDialog : public QDialog
{
  Q_OBJECT

public:

  ParameterColorDialog(QWidget *parent = 0);
  virtual ~ParameterColorDialog();
  bool getChanges();

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

};

#endif

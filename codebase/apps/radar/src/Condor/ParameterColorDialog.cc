
#include <QtWidgets>
#include <QPixmap>

#include "FlowLayout.hh"
#include "ParameterColorDialog.hh"
#include "ClickableLabel.hh"
#include "../Condor/ColorMap.hh"
#include "../Condor/ColorBar.hh"

ParameterColorDialog::ParameterColorDialog(QWidget *parent) :
    QDialog(parent)
{
    // setTitle("Parameter + Color");
    QGridLayout *layout = new QGridLayout;
    setLayout(layout);

    // Parameter list 
    
    QLabel *parameterLabel = new QLabel();
    parameterLabel->setText(tr("Parameters"));

    QListWidget *parameterList = new QListWidget(this);
    for (int i=0; i<20; i++)    // TODO: get a real list of the field Names?? 
      parameterList->addItem("DBZ");

    // Color Palette Editor widget creations ...
    QPushButton *cancelButton = new QPushButton("Cancel");
    QPushButton *saveButton = new QPushButton(tr("Save"));
    QPushButton *replotButton = new QPushButton(tr("Replot"));

    centerColorLabel = new QLabel;
    centerColorLabel->setText(tr("Center"));
    centerColorLineEdit = new QLineEdit();
    
    minColorLabel = new QLabel;
    minColorLabel->setText(tr("Min"));
    minColorLineEdit = new QLineEdit();

    maxColorLabel = new QLabel;
    maxColorLabel->setText(tr("Max"));
    maxColorLineEdit = new QLineEdit();

    stepColorLabel = new QLabel;
    stepColorLabel->setText(tr("Step"));
    stepColorLineEdit = new QLineEdit();

    gridColorLabel = new QLabel;
    gridColorLabel->setText(tr("Grid"));
    gridColorButton = new QPushButton(tr(""));
    gridColorButton->setFlat(true);
    setColorOnButton(gridColorButton, QColor("white"));

    boundaryColorLabel = new QLabel;
    boundaryColorLabel->setText(tr("Boundary"));
    boundaryColorButton = new QPushButton(tr(""));
    boundaryColorButton->setFlat(true);
    setColorOnButton(boundaryColorButton, QColor("blue"));

    exceededColorLabel = new QLabel;
    exceededColorLabel->setText(tr("Exceeded"));
    exceededColorButton = new QPushButton(tr(""));
    exceededColorButton->setFlat(true);
    setColorOnButton(exceededColorButton, QColor("black"));

    missingColorLabel = new QLabel;
    missingColorLabel->setText(tr("Missing"));
    missingColorButton = new QPushButton(tr(""));
    missingColorButton->setFlat(true);
    setColorOnButton(missingColorButton, QColor("black"));

    annotationColorLabel = new QLabel;
    annotationColorLabel->setText(tr("Annotation"));
    annotationColorButton = new QPushButton(tr(""));
    annotationColorButton->setFlat(true);
    setColorOnButton(annotationColorButton, QColor("white"));

    backgroundColorLabel = new QLabel;
    backgroundColorLabel->setText(tr("Background"));
    backgroundColorButton = new QPushButton(tr(""));
    backgroundColorButton->setFlat(true);
    setColorOnButton(backgroundColorButton, QColor("grey"));

    emphasisColorLabel = new QLabel;
    emphasisColorLabel->setText(tr("Emphasis"));
    emphasisColorButton = new QPushButton(tr(""));
    emphasisColorButton->setFlat(true);
    setColorOnButton(emphasisColorButton, QColor("pink"));

    QLabel *paletteLabel = new QLabel();
    paletteLabel->setText(tr("Palette"));

    ColorMap *cmap = new ColorMap(0.0, 100.0, "default");
    ColorBar *colorBar = new ColorBar(1, cmap); // , this);
    QPixmap *pixmap = colorBar->getPixmap();
    ClickableLabel *cmapLabel = new ClickableLabel();
    cmapLabel->clear();
    cmapLabel->setPixmap(*pixmap);

    setValueOnLineEdit(minColorLineEdit, cmap->rangeMin());
    setValueOnLineEdit(maxColorLineEdit, cmap->rangeMax());

    // end Color Palette Editor


    connect(cancelButton, &QAbstractButton::clicked, this, &ParameterColorDialog::cancelColorScale);
    connect(saveButton, &QAbstractButton::clicked, this, &ParameterColorDialog::saveColorScale);
    connect(replotButton, &QAbstractButton::clicked, this, &ParameterColorDialog::replotColorScale);
   
    connect(gridColorButton, &QAbstractButton::clicked, this, &ParameterColorDialog::setGridColor);
    connect(boundaryColorButton, &QAbstractButton::clicked, this, &ParameterColorDialog::setBoundaryColor);
    connect(exceededColorButton, &QAbstractButton::clicked, this, &ParameterColorDialog::setExceededColor);
    connect(missingColorButton, &QAbstractButton::clicked, this, &ParameterColorDialog::setMissingColor);
    connect(annotationColorButton, &QAbstractButton::clicked, this, &ParameterColorDialog::setAnnotationColor);
    connect(backgroundColorButton, &QAbstractButton::clicked, this, &ParameterColorDialog::setBackgroundColor);
    connect(emphasisColorButton, &QAbstractButton::clicked, this, &ParameterColorDialog::setEmphasisColor);

    connect(cmapLabel, &ClickableLabel::clicked, this, &ParameterColorDialog::pickColorPalette);

    layout->addWidget(parameterLabel, 0, 0, 1, 1, Qt::AlignCenter);
    layout->addWidget(parameterList, 1, 0, 8, 1);

    layout->addWidget(centerColorLabel, 0, 1);
    layout->addWidget(centerColorLineEdit, 0, 2);
    layout->addWidget(maxColorLabel, 1, 1);
    layout->addWidget(maxColorLineEdit, 1, 2);
    layout->addWidget(minColorLabel, 2, 1);
    layout->addWidget(minColorLineEdit, 2, 2);
    layout->addWidget(stepColorLabel, 3, 1);
    layout->addWidget(stepColorLineEdit, 3, 2);

    int base = 4;
    layout->addWidget(gridColorButton, base+0, 2);
    layout->addWidget(gridColorLabel, base+0, 1);
    layout->addWidget(boundaryColorButton, base+1, 2);
    layout->addWidget(boundaryColorLabel, base+1, 1);
    layout->addWidget(exceededColorButton, base+2, 2);
    layout->addWidget(exceededColorLabel, base+2, 1);
    layout->addWidget(missingColorButton, base+3, 2);
    layout->addWidget(missingColorLabel, base+3, 1);
    layout->addWidget(annotationColorButton, base+4, 2);
    layout->addWidget(annotationColorLabel, base+4, 1);
    layout->addWidget(backgroundColorButton, base+5, 2);
    layout->addWidget(backgroundColorLabel, base+5, 1);
    layout->addWidget(emphasisColorButton, base+6, 2);
    layout->addWidget(emphasisColorLabel, base+6, 1);
    layout->addWidget(saveButton, base+7, 2);
    layout->addWidget(cancelButton, base+7, 1);
    layout->addWidget(replotButton, base+7, 0);
//    layout->addWidget(saveButton, 1, 0);

    // add the palette

    layout->addWidget(paletteLabel, 0, 3, 1, 1, Qt::AlignCenter);
    layout->addWidget(cmapLabel, 1, 3, -1, 1);

    setLayout(layout);
}


ParameterColorDialog::~ParameterColorDialog() {} 

void ParameterColorDialog::cancelColorScale() {

  QString qvalue;
  bool ok = false;
/*
  qvalue = centerColorLineEdit->text();
  double value = qvalue.toDouble(&ok);

  if (ok) {
    printf("value entered %g\n", value);
  } else {
    printf("unrecognized value entered\n");
  }
*/
  QMessageBox msgBox;
  msgBox.setText("Cancel Clicked!");
  msgBox.exec();
}

/*
void ParameterColorDialog::saveColorScale() {

  QString qvalue;
  bool ok = false;
//
  qvalue = centerColorLineEdit->text();
  double value = qvalue.toDouble(&ok);

  if (ok) {
    printf("value entered %g\n", value);
  } else {
    printf("unrecognized value entered\n");
  }
//
  QMessageBox msgBox;
  msgBox.setText("Save Clicked!");
  msgBox.exec();
}
*/

bool ParameterColorDialog::getChanges() {

   //show();
   return false;

}

void ParameterColorDialog::setCenterPoint()
{
  QString qvalue;
  bool ok = false;
  qvalue = centerColorLineEdit->text();
  double value = qvalue.toDouble(&ok);
        
  if (ok) {
    printf("value entered %g\n", value);
  } else {
    printf("unrecognized value entered\n");
  } 
  //qvalue = centerColorLineEdit->text();
  //std::string value = qvalue.toStdString();
  //sscanf(value.c_str(),"%g", &newCenterPoint);
}       

void ParameterColorDialog::setMaxPoint()
{
  QString qvalue;
  bool ok = false;
  qvalue = maxColorLineEdit->text();
  double value = qvalue.toDouble(&ok);

  if (ok) {
    printf("value entered %g\n", value);
  } else {
    printf("unrecognized value entered\n");
  }
  //qvalue = centerColorLineEdit->text();
  //std::string value = qvalue.toStdString();
  //sscanf(value.c_str(),"%g", &newCenterPoint);
}

void ParameterColorDialog::setMinPoint()
{
  QString qvalue;
  bool ok = false;
  qvalue = minColorLineEdit->text();
  double value = qvalue.toDouble(&ok);

  if (ok) {
    printf("value entered %g\n", value);
  } else {
    printf("unrecognized value entered\n");
  }
  //qvalue = centerColorLineEdit->text();
  //std::string value = qvalue.toStdString();
  //sscanf(value.c_str(),"%g", &newCenterPoint);
}

void ParameterColorDialog::setStepPoint()
{
  QString qvalue;
  bool ok = false;
  qvalue = stepColorLineEdit->text();
  double value = qvalue.toDouble(&ok);

  if (ok) {
    printf("value entered %g\n", value);
  } else {
    printf("unrecognized value entered\n");
  }
  //qvalue = centerColorLineEdit->text();
  //std::string value = qvalue.toStdString();
  //sscanf(value.c_str(),"%g", &newCenterPoint);
}

void ParameterColorDialog::saveColorScale() {

  QString qvalue;
  bool ok = false;
  qvalue = centerColorLineEdit->text();
  double value = qvalue.toDouble(&ok);
    
  if (ok) { 
    printf("value entered %g\n", value); 
  } else { 
    printf("unrecognized value entered\n");
  }

}

void ParameterColorDialog::replotColorScale() {

  QString qvalue;
  bool ok = false;

  QMessageBox msgBox;
  msgBox.setText("Replot Clicked!");
  msgBox.exec();
}

void ParameterColorDialog::setGridColor()
{   
    // const QColorDialog::ColorDialogOptions options = QFlag(colorDialogOptionsWidget->value());
    const QColor color = QColorDialog::getColor(Qt::green, this); // , "Select Color", options);

    if (color.isValid()) {
        gridColorButton->setPalette(QPalette(color));
        gridColorButton->setAutoFillBackground(true);
    }
}

void ParameterColorDialog::setBoundaryColor()
{
    // const QColorDialog::ColorDialogOptions options = QFlag(colorDialogOptionsWidget->value());
    const QColor color = QColorDialog::getColor(Qt::green, this); // , "Select Color", options);

    if (color.isValid()) {
        boundaryColorButton->setPalette(QPalette(color));
        boundaryColorButton->setAutoFillBackground(true);
    }
}

void ParameterColorDialog::setExceededColor()
{
    // const QColorDialog::ColorDialogOptions options = QFlag(colorDialogOptionsWidget->value());
    const QColor color = QColorDialog::getColor(Qt::green, this); // , "Select Color", options);

    if (color.isValid()) {
        exceededColorButton->setPalette(QPalette(color));
        exceededColorButton->setAutoFillBackground(true);
    }
}

void ParameterColorDialog::setMissingColor()
{
    // const QColorDialog::ColorDialogOptions options = QFlag(colorDialogOptionsWidget->value());
    const QColor color = QColorDialog::getColor(Qt::green, this); // , "Select Color", options);

    if (color.isValid()) {
        missingColorButton->setPalette(QPalette(color));
        missingColorButton->setAutoFillBackground(true);
    }
}

void ParameterColorDialog::setAnnotationColor()
{
    // const QColorDialog::ColorDialogOptions options = QFlag(colorDialogOptionsWidget->value());
    const QColor color = QColorDialog::getColor(Qt::green, this); // , "Select Color", options);

    if (color.isValid()) {
        annotationColorButton->setPalette(QPalette(color));
        annotationColorButton->setAutoFillBackground(true);
    }
}

void ParameterColorDialog::setBackgroundColor()
{
    // const QColorDialog::ColorDialogOptions options = QFlag(colorDialogOptionsWidget->value());
    const QColor color = QColorDialog::getColor(Qt::green, this); // , "Select Color", options);

    if (color.isValid()) {
        backgroundColorButton->setPalette(QPalette(color));
        backgroundColorButton->setAutoFillBackground(true);
    }
}

void ParameterColorDialog::setColorOnButton(QPushButton *button, QColor color)
{
    if (color.isValid()) {
        button->setPalette(QPalette(color));
        button->setAutoFillBackground(true);
    }
}

void ParameterColorDialog::setEmphasisColor()
{
    // const QColorDialog::ColorDialogOptions options = QFlag(colorDialogOptionsWidget->value());
    const QColor color = QColorDialog::getColor(Qt::green, this); // , "Select Color", options);

    if (color.isValid()) {
        emphasisColorButton->setPalette(QPalette(color));
        emphasisColorButton->setAutoFillBackground(true);
    }
}

void ParameterColorDialog::setValueOnLineEdit(QLineEdit *editor, double value) {
    QString svalue = QString::number(value);
    editor->setText(QString(svalue));
}

void ParameterColorDialog::pickColorPalette()
{
    ColorMapTemplates colorMapTemplates(this);
    colorMapTemplates.exec();
    // bool changed = parameterColorDialog.getChanges();
}

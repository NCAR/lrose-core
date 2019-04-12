
#include <QtWidgets>
#include <QPixmap>
#include <QListWidget>
#include <QListWidgetItem>

#include "FlowLayout.hh"
#include "ParameterColorView.hh"
// #include "ClickableLabel.hh"
#include "toolsa/LogStream.hh"
#include "ColorMap.hh"
#include "ColorBar.hh"

ParameterColorView::ParameterColorView(QWidget *parent) :
    QDialog(parent)
{
  _parent = parent;
  currentColorMap = NULL;
  colorBar = NULL;
  cmapLabel = NULL;
  _selectedField = "";
}

// TODO: divide this into two methods
// fieldSelected
// fieldNamesChanged



void ParameterColorView::updateEvent(vector<string> fieldNames, string selectedField)
//				map<string, string> selectedColorMap)
{
  LOG(DEBUG) << "entry";

    // setTitle("Parameter + Color");
    layout = new QGridLayout;
    setLayout(layout);

    // We can also select a field for view in the current display
    // set the selected field
    // display the color map for the selected field
    // Parameter list 
    
    QLabel *parameterLabel = new QLabel();
    parameterLabel->setText(tr("Fields"));

    QListWidget *parameterList = new QListWidget(this);
   

    // move this code to SLOT(DisplayFieldsSupplied)
    //  iterator over fieldNames
    int selectedIndex = -1;
    int idx = 0;
    for (vector<string>::iterator fieldName = fieldNames.begin(); fieldName != fieldNames.end(); fieldName++) {
     
      parameterList->addItem(QString::fromStdString(*fieldName));
      if (selectedField.compare(*fieldName) == 0) {
	//        selectedIndex = idx;
	parameterList->setCurrentRow(idx);
      }
      idx++;
    }

  /* if (selectedIndex >= 0) {

    //    QListWidgetItem *selectedQListWidgetItem = parameterList->itemFromIndex(selectedIndex);
    parameterList->setCurrentItem(selectedQListWidgetItem);
  }
  */

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

    QString maxInputMask("####.#");
    minColorLineEdit->setInputMask(maxInputMask);

    maxColorLabel = new QLabel;
    maxColorLabel->setText(tr("Max"));
    maxColorLineEdit = new QLineEdit();
    maxColorLineEdit->setInputMask(maxInputMask);

    stepColorLabel = new QLabel;
    stepColorLabel->setText(tr("Step"));
    stepColorLineEdit = new QLineEdit();

    gridColorLabel = new QLabel;
    gridColorLabel->setText(tr("Grid"));
    gridColorButton = new QPushButton(tr(""));
    gridColorButton->setFlat(true);
    setColorOnButton(gridColorButton, QColor("white"));

    // TODO: how to get the color map data from the model?    setColorOnButton(gridColorButton, QColor("white"));
    // Send a ColorMap object? or individual values?

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

    // TODO: call separate method here ..

    /*    
    currentColorMap = new ColorMap(0.0, 100.0, "default");
    colorBar = new ColorBar(1, currentColorMap); // , this);
    QPixmap *pixmap = colorBar->getPixmap();
    cmapLabel = new ClickableLabel();
    cmapLabel->clear();
    cmapLabel->setPixmap(*pixmap);

    setValueOnLineEdit(minColorLineEdit, currentColorMap->rangeMin());
    setValueOnLineEdit(maxColorLineEdit, currentColorMap->rangeMax());
    */

    // end Color Palette Editor

    connect(parameterList, SIGNAL(currentItemChanged(QListWidgetItem *, QListWidgetItem *)),
	    this, SLOT(fieldSelected(QListWidgetItem *, QListWidgetItem *))); 
    connect(cancelButton, &QAbstractButton::clicked, this, &ParameterColorView::cancelColorScale);
    connect(saveButton, &QAbstractButton::clicked, this, &ParameterColorView::saveColorScale);
    connect(replotButton, &QAbstractButton::clicked, this, &ParameterColorView::replotColorScale);
   
    connect(gridColorButton, &QAbstractButton::clicked, this, &ParameterColorView::setGridColor);
    connect(boundaryColorButton, &QAbstractButton::clicked, this, &ParameterColorView::setBoundaryColor);
    connect(exceededColorButton, &QAbstractButton::clicked, this, &ParameterColorView::setExceededColor);
    connect(missingColorButton, &QAbstractButton::clicked, this, &ParameterColorView::setMissingColor);
    connect(annotationColorButton, &QAbstractButton::clicked, this, &ParameterColorView::setAnnotationColor);
    connect(backgroundColorButton, &QAbstractButton::clicked, this, &ParameterColorView::setBackgroundColor);
    connect(emphasisColorButton, &QAbstractButton::clicked, this, &ParameterColorView::setEmphasisColor);

    connect(maxColorLineEdit, &QLineEdit::editingFinished, this, &ParameterColorView::setMaxPoint);
    connect(minColorLineEdit, &QLineEdit::editingFinished, this, &ParameterColorView::setMinPoint);

    //    connect(cmapLabel, &ClickableLabel::clicked, this, &ParameterColorView::pickColorPalette);

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

    // kick off the fetch of the color map
  LOG(DEBUG) << "selected Field is ...";
  LOG(DEBUG) << selectedField;

  //_selectedField = selectedField;
  emit getColorMap(selectedField);

  
  LOG(DEBUG) << "exit";

}


ParameterColorView::~ParameterColorView() {} 

void ParameterColorView::errorMessage(string title, string message) {
  QMessageBox::information(this, QString::fromStdString(title), QString::fromStdString(message));
}



void ParameterColorView::fieldSelected(QListWidgetItem *current, QListWidgetItem *previous) {
  LOG(DEBUG) << "enter";
  // get the color map for the current field
  QString fieldName = current->text();
  
  string newSelection = fieldName.toStdString();
  if (newSelection.compare(_selectedField) == 0) {
    // they are the same, no need for changes
  } else {
    _selectedField = newSelection;
    emit getColorMap(_selectedField);
  } 
  LOG(DEBUG) << "exit";
}

void ParameterColorView::colorMapProvided(string fieldName, ColorMap *colorMap) {
  LOG(DEBUG) << "entry";
  if (colorMap == NULL) {
    errorMessage("field not found", fieldName);
  } else {
  // delete and free the previous color map
    //if (currentColorMap != NULL) delete currentColorMap;
    if (colorBar != NULL) delete colorBar;
    if (cmapLabel != NULL) delete cmapLabel;

  currentColorMap = colorMap;
  colorBar = new ColorBar(1, colorMap);
  QPixmap *pixmap = colorBar->getPixmap();
  cmapLabel = new ClickableLabel();
  cmapLabel->clear();
  cmapLabel->setPixmap(*pixmap);

  setValueOnLineEdit(minColorLineEdit, currentColorMap->rangeMin());
  setValueOnLineEdit(maxColorLineEdit, currentColorMap->rangeMax());

  connect(cmapLabel, &ClickableLabel::clicked, this, &ParameterColorView::pickColorPalette);

    layout->addWidget(cmapLabel, 1, 3, -1, 1);

    //setLayout(layout);


  }
  LOG(DEBUG) << "exit";
}

void ParameterColorView::cancelColorScale() {

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
void ParameterColorView::saveColorScale() {

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

bool ParameterColorView::getChanges() {

   //show();
   return false;

}

void ParameterColorView::setCenterPoint()
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

void ParameterColorView::setMaxPoint()
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

  // modify the working colorMap
  emit colorMapMaxChanged(value);

  //qvalue = centerColorLineEdit->text();
  //std::string value = qvalue.toStdString();
  //sscanf(value.c_str(),"%g", &newCenterPoint);
}

void ParameterColorView::setMinPoint()
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

  // modify the working colorMap
  emit colorMapMinChanged(value);

  //qvalue = centerColorLineEdit->text();
  //std::string value = qvalue.toStdString();
  //sscanf(value.c_str(),"%g", &newCenterPoint);
}

void ParameterColorView::setStepPoint()
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

void ParameterColorView::saveColorScale() {

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

void ParameterColorView::replotColorScale() {

  QString qvalue;
  bool ok = false;

  emit replot(_selectedField);

  QMessageBox msgBox;
  msgBox.setText("Replot Clicked!");
  msgBox.exec();
}

void ParameterColorView::setGridColor()
{   
    // const QColorView::ColorDialogOptions options = QFlag(colorDialogOptionsWidget->value());
    const QColor color = QColorDialog::getColor(Qt::green, this); // , "Select Color", options);

    if (color.isValid()) {
        gridColorButton->setPalette(QPalette(color));
        gridColorButton->setAutoFillBackground(true);
    }
}

void ParameterColorView::setBoundaryColor()
{
    // const QColorDialog::ColorDialogOptions options = QFlag(colorDialogOptionsWidget->value());
    const QColor color = QColorDialog::getColor(Qt::green, this); // , "Select Color", options);

    if (color.isValid()) {
        boundaryColorButton->setPalette(QPalette(color));
        boundaryColorButton->setAutoFillBackground(true);
    }
}

void ParameterColorView::setExceededColor()
{
    // const QColorDialog::ColorDialogOptions options = QFlag(colorDialogOptionsWidget->value());
    const QColor color = QColorDialog::getColor(Qt::green, this); // , "Select Color", options);

    if (color.isValid()) {
        exceededColorButton->setPalette(QPalette(color));
        exceededColorButton->setAutoFillBackground(true);
    }
}

void ParameterColorView::setMissingColor()
{
    // const QColorDialog::ColorDialogOptions options = QFlag(colorDialogOptionsWidget->value());
    const QColor color = QColorDialog::getColor(Qt::green, this); // , "Select Color", options);

    if (color.isValid()) {
        missingColorButton->setPalette(QPalette(color));
        missingColorButton->setAutoFillBackground(true);
    }
}

void ParameterColorView::setAnnotationColor()
{
    // const QColorDialog::ColorDialogOptions options = QFlag(colorDialogOptionsWidget->value());
    const QColor color = QColorDialog::getColor(Qt::green, this); // , "Select Color", options);

    if (color.isValid()) {
        annotationColorButton->setPalette(QPalette(color));
        annotationColorButton->setAutoFillBackground(true);
    }
}

void ParameterColorView::setBackgroundColor()
{
    // const QColorDialog::ColorDialogOptions options = QFlag(colorDialogOptionsWidget->value());
    const QColor color = QColorDialog::getColor(Qt::green, this); // , "Select Color", options);

    if (color.isValid()) {
        backgroundColorButton->setPalette(QPalette(color));
        backgroundColorButton->setAutoFillBackground(true);
    }
}

void ParameterColorView::setColorOnButton(QPushButton *button, QColor color)
{
    if (color.isValid()) {
        button->setPalette(QPalette(color));
        button->setAutoFillBackground(true);
    }
}

void ParameterColorView::setEmphasisColor()
{
    // const QColorDialog::ColorDialogOptions options = QFlag(colorDialogOptionsWidget->value());
    const QColor color = QColorDialog::getColor(Qt::green, this); // , "Select Color", options);

    if (color.isValid()) {
        emphasisColorButton->setPalette(QPalette(color));
        emphasisColorButton->setAutoFillBackground(true);
    }
}

void ParameterColorView::setValueOnLineEdit(QLineEdit *editor, double value) {
    QString svalue = QString::number(value);
    editor->setText(QString(svalue));
}

void ParameterColorView::pickColorPalette()
{
    ColorMapTemplates colorMapTemplates(this);
    colorMapTemplates.exec();
    // bool changed = parameterColorDialog.getChanges();
}

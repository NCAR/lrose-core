
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
  resize(200,300);
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
    // int selectedIndex = -1;
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
    // TODO: consider making the PushButtons Actions instead and put them in a toolbar??
    //QAction *cancelButton = new QAction("&Cancel", this);
    cancelButton->setIcon(QIcon(":/images/cancel_x.png"));
    QPushButton *saveButton = new QPushButton(tr("Save"));
    QPushButton *replotButton = new QPushButton(tr("Apply"));
    replotButton->setIcon(QIcon(":/images/apply.png"));  // apply changes to selected field
    // Note: Command buttons in dialogs are by default auto-default buttons.
    // A default button is a push button that is activated when the user 
    // presses the Enter or Return key in a dialog.
    // We only want the replot button to be the default button,
    // therefore, setAutoDefault(false) for all the other pushButtons.
    // Otherwise, when Enter/Return is pressed, the color buttons
    // will be activated and the colorDialog chooser pops-up.
    cancelButton->setAutoDefault(false);
    saveButton->setAutoDefault(false);
    replotButton->setAutoDefault(false);

    centerColorLabel = new QLabel;
    centerColorLabel->setText(tr("Center"));
    centerColorLineEdit = new QLineEdit();
    //centerColorLineEdit->setAutoDefault(false);
    
    minColorLabel = new QLabel;
    minColorLabel->setText(tr("Min"));
    minColorLineEdit = new QLineEdit();
    //minColorLineEdit->setAutoDefault(false);

    QString maxInputMask("####.#");
    //minColorLineEdit->setInputMask(maxInputMask);

    maxColorLabel = new QLabel;
    maxColorLabel->setText(tr("Max"));
    maxColorLineEdit = new QLineEdit();
    //maxColorLineEdit->setInputMask(maxInputMask);
    //maxColorLineEdit->setAutoDefault(false);

    stepColorLabel = new QLabel;
    stepColorLabel->setText(tr("Step"));
    stepColorLineEdit = new QLineEdit();
    //stepColorLineEdit->setAutoDefault(false);

    gridColorLabel = new QLabel;
    gridColorLabel->setText(tr("Grid"));
    gridColorButton = new QPushButton(tr(""));
    gridColorButton->setFlat(true);
    gridColorButton->setAutoDefault(false);
    setColorOnButton(gridColorButton, QColor("white"));

    // TODO: how to get the color map data from the model?    setColorOnButton(gridColorButton, QColor("white"));
    // Send a ColorMap object? or individual values?

    boundaryColorLabel = new QLabel;
    boundaryColorLabel->setText(tr("Boundary"));
    boundaryColorButton = new QPushButton(tr(""));
    boundaryColorButton->setFlat(true);
    boundaryColorButton->setAutoDefault(false);
    setColorOnButton(boundaryColorButton, QColor("blue"));

    exceededColorLabel = new QLabel;
    exceededColorLabel->setText(tr("Exceeded"));
    exceededColorButton = new QPushButton(tr(""));
    exceededColorButton->setAutoDefault(false);
    exceededColorButton->setFlat(true);
    setColorOnButton(exceededColorButton, QColor("black"));

    missingColorLabel = new QLabel;
    missingColorLabel->setText(tr("Missing"));
    missingColorButton = new QPushButton(tr(""));
    missingColorButton->setFlat(true);
    missingColorButton->setAutoDefault(false);
    setColorOnButton(missingColorButton, QColor("black"));

    annotationColorLabel = new QLabel;
    annotationColorLabel->setText(tr("Annotation"));
    annotationColorButton = new QPushButton(tr(""));
    annotationColorButton->setFlat(true);
    annotationColorButton->setAutoDefault(false);
    setColorOnButton(annotationColorButton, QColor("white"));

    backgroundColorLabel = new QLabel;
    backgroundColorLabel->setText(tr("Background"));
    backgroundColorButton = new QPushButton(tr(""));
    backgroundColorButton->setFlat(true);
    backgroundColorButton->setAutoDefault(false);
    setColorOnButton(backgroundColorButton, QColor("grey"));

    emphasisColorLabel = new QLabel;
    emphasisColorLabel->setText(tr("Emphasis"));
    emphasisColorButton = new QPushButton(tr(""));
    emphasisColorButton->setFlat(true);
    emphasisColorButton->setAutoDefault(false);
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
    //connect(cancelButton, &QAction::triggered, this, &ParameterColorView::cancelColorScale);
    //connect(saveButton, &QAbstractButton::clicked, this, &ParameterColorView::replotColorScale); // saveColorScale);
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
    // layout->addWidget(saveButton, base+7, 2);
    layout->addWidget(cancelButton, base+7, 1);
    //layout->addAction(cancelButton, base+7, 1);
    layout->addWidget(replotButton, base+7, 0);
//    layout->addWidget(saveButton, 1, 0);

    // add the palette

    layout->addWidget(paletteLabel, 0, 3, 1, 1, Qt::AlignCenter);
    layout->addWidget(cmapLabel, 1, 3, -1, 1);

    setLayout(layout);

    // kick off the fetch of the color map
  LOG(DEBUG) << "selected Field is ...";
  LOG(DEBUG) << selectedField;

  _selectedField = selectedField;
  emit getColorMap(selectedField);
  emit getGridColor();
  emit getEmphasisColor();
  emit getAnnotationColor();
  emit getBackgroundColor();
  
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
  LOG(DEBUG) << "selectedField is now " << _selectedField;
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
  QPixmap *pixmap = colorBar->getPixmap(1,3);
  cmapLabel = new ClickableLabel();
  // int w = cmapLabel->width();
  // int h = cmapLabel->height();

  cmapLabel->clear();
  cmapLabel->setPixmap(*pixmap); //
  cmapLabel->show();
  //QSize size;
  //size.setWidth(1); 
  // this doesn't allow scaling of image ... (pixmap->scaled(w/2,h*2)); // (*pixmap);
  //cmapLabel->sizeHint = size;
  setValueOnLineEdit(minColorLineEdit, currentColorMap->rangeMin());
  setValueOnLineEdit(maxColorLineEdit, currentColorMap->rangeMax());

  connect(cmapLabel, &ClickableLabel::clicked, this, &ParameterColorView::pickColorPalette);

    layout->addWidget(cmapLabel, 1, 3, -1, 1);

  }
  LOG(DEBUG) << "exit";
}

void ParameterColorView::gridColorProvided(QColor color) {
  setColorOnButton(gridColorButton, color);
}

void ParameterColorView::emphasisColorProvided(QColor color) {
  setColorOnButton(emphasisColorButton, color);
}

void ParameterColorView::annotationColorProvided(QColor color) {
  setColorOnButton(annotationColorButton, color);
}

void ParameterColorView::backgroundColorProvided(QColor color) {
  setColorOnButton(backgroundColorButton, color);
}

void ParameterColorView::cancelColorScale() {

  close(); 
  /*
  QMessageBox msgBox;
  msgBox.setText("Cancel Clicked!");
  msgBox.exec();
  */
}


void ParameterColorView::saveColorScale() {

  QString qvalue;
  // bool ok = false;

  QMessageBox msgBox;
  msgBox.setText("Not Implemented");
  msgBox.exec();
}


bool ParameterColorView::getChanges() {

   //show();
   return false;

}

void ParameterColorView::setCenterPoint()
{
  LOG(DEBUG) << "enter";
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
  LOG(DEBUG) << "exit";

}       

void ParameterColorView::setMaxPoint()
{
  LOG(DEBUG) << "enter";

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
  LOG(DEBUG) << "enter";

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
  LOG(DEBUG) << "enter";

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

/*
void ParameterColorView::saveColorScale() {
  LOG(DEBUG) << "enter";

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
*/

void ParameterColorView::replotColorScale() {

  LOG(DEBUG) << "enter";

  QString qvalue;
  // bool ok = false;
  
  LOG(DEBUG) << "enter";
  LOG(DEBUG) << "emit replotFieldColorMapChanges";

  emit replotFieldColorMapChanges(); // _selectedField);
/*
  QMessageBox msgBox;
  msgBox.setText("Replot ...");
  msgBox.exec();
*/
  LOG(DEBUG) << "exit";
}

void ParameterColorView::setGridColor()
{   
  LOG(DEBUG) << "enter";

  
    // const QColorView::ColorDialogOptions options = QFlag(colorDialogOptionsWidget->value());
    const QColor color = QColorDialog::getColor(Qt::green, this); // , "Select Color", options);

    if (color.isValid()) {
        gridColorButton->setPalette(QPalette(color));
        gridColorButton->setAutoFillBackground(true);
        emit gridColorChanged(color);
    }
  
  LOG(DEBUG) << "exit";
}

void ParameterColorView::setBoundaryColor()
{
  LOG(DEBUG) << "enter";

  errorMessage("", "Not Implemented");
    /*

    // const QColorDialog::ColorDialogOptions options = QFlag(colorDialogOptionsWidget->value());
    const QColor color = QColorDialog::getColor(Qt::green, this); // , "Select Color", options);

    if (color.isValid()) {
        boundaryColorButton->setPalette(QPalette(color));
        boundaryColorButton->setAutoFillBackground(true);
	//         emit boundaryColorChanged(color);
    }
    */
  LOG(DEBUG) << "exit";

}

void ParameterColorView::setExceededColor()
{
  LOG(DEBUG) << "enter";

  errorMessage("", "Not Implemented");
  /*
    // const QColorDialog::ColorDialogOptions options = QFlag(colorDialogOptionsWidget->value());
    const QColor color = QColorDialog::getColor(Qt::green, this); // , "Select Color", options);

    if (color.isValid()) {
        exceededColorButton->setPalette(QPalette(color));
        exceededColorButton->setAutoFillBackground(true);
    }
  */
  LOG(DEBUG) << "exit";

}

void ParameterColorView::setMissingColor()
{
  LOG(DEBUG) << "enter";
  errorMessage("", "Not Implemented");
  /*
    // const QColorDialog::ColorDialogOptions options = QFlag(colorDialogOptionsWidget->value());
    const QColor color = QColorDialog::getColor(Qt::green, this); // , "Select Color", options);

    if (color.isValid()) {
        missingColorButton->setPalette(QPalette(color));
        missingColorButton->setAutoFillBackground(true);
    }
  */
  LOG(DEBUG) << "exit";

}

void ParameterColorView::setAnnotationColor()
{
  LOG(DEBUG) << "enter";

  errorMessage("", "Not Implemented");
  /*

    // const QColorDialog::ColorDialogOptions options = QFlag(colorDialogOptionsWidget->value());
    const QColor color = QColorDialog::getColor(Qt::green, this); // , "Select Color", options);

    if (color.isValid()) {
        annotationColorButton->setPalette(QPalette(color));
        annotationColorButton->setAutoFillBackground(true);
    }
  */
  LOG(DEBUG) << "exit";

}

void ParameterColorView::setBackgroundColor()
{
  LOG(DEBUG) << "enter";

    // const QColorDialog::ColorDialogOptions options = QFlag(colorDialogOptionsWidget->value());
    const QColor color = QColorDialog::getColor(Qt::green, this); // , "Select Color", options);

    if (color.isValid()) {
        backgroundColorButton->setPalette(QPalette(color));
        backgroundColorButton->setAutoFillBackground(true);
        emit backgroundColorChanged(color);
    }
  LOG(DEBUG) << "exit";

}

void ParameterColorView::setColorOnButton(QPushButton *button, QColor color)
{
  LOG(DEBUG) << "enter";

    if (color.isValid()) {
        button->setPalette(QPalette(color));
        button->setAutoFillBackground(true);
    }
  LOG(DEBUG) << "exit";

}

void ParameterColorView::setEmphasisColor()
{
  LOG(DEBUG) << "enter";
  errorMessage("", "Not Implemented");

  /*
    // const QColorDialog::ColorDialogOptions options = QFlag(colorDialogOptionsWidget->value());
    const QColor color = QColorDialog::getColor(Qt::green, this); // , "Select Color", options);

    if (color.isValid()) {
        emphasisColorButton->setPalette(QPalette(color));
        emphasisColorButton->setAutoFillBackground(true);
    }
  */
  LOG(DEBUG) << "exit";

}

void ParameterColorView::setValueOnLineEdit(QLineEdit *editor, double value) {
    QString svalue = QString::number(value);
    editor->setText(QString(svalue));
}

void ParameterColorView::pickColorPalette()
{
  emit pickColorPaletteRequest();

  // ColorMapTemplates colorMapTemplates(this);
  //  colorMapTemplates.exec();
    // bool changed = parameterColorDialog.getChanges();
}

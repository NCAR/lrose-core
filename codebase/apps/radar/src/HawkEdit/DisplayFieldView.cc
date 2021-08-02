#include "DisplayFieldView.hh"
#include <toolsa/LogStream.hh>

#include <QMenu>
#include <QMessageBox>

DisplayFieldView::DisplayFieldView() { // DisplayFieldController *displayFieldController) {
  //_controller = displayFieldController;
  _haveFilteredFields = false;
  _label_font_size = 12;
  //createFieldPanel(parent);
}

DisplayFieldView::~DisplayFieldView() {

}

//QWidget *DisplayFieldView::getViewWidget() {
//  return this;
//}

void DisplayFieldView::set_label_font_size(int size) {
  _label_font_size = size;
}

void DisplayFieldView::setHaveFilteredFields(bool value) {
  _haveFilteredFields = value;
}

//////////////////////////////////////////////
// create the field panel

void DisplayFieldView::createFieldPanel(QWidget *parent)
{
  LOG(DEBUG) << "enter";
  //ParamFile *_params = ParamFile::Instance();

  Qt::Alignment alignCenter(Qt::AlignCenter);
  Qt::Alignment alignRight(Qt::AlignRight);
  
  int fsize = _label_font_size;
  int fsize2 = _label_font_size + 2;
  int fsize4 = _label_font_size + 4;
  int fsize6 = _label_font_size + 6;

  //this = new QGroupBox(parent); // <=== here is the connection
  _fieldGroup = new QButtonGroup;
  _fieldsLayout = new QVBoxLayout(this);
  //_fieldsLayout->setVerticalSpacing(5);

  int row = 0;
  int nCols = 3;
  if (_haveFilteredFields) {
    nCols = 4;
  }
/*
  _displayFieldController->setSelectedField(0);
  _selectedField = _displayFieldController->getSelectedField(); //_fields[0];
  _selectedLabel = _selectedField->getLabel(); //_fields[0]->getLabel();
  _selectedName = _selectedField->getName(); // _fields[0]->getName();
  //  _selectedField = _fields[0];
  //_selectedLabel = _fields[0]->getLabel();
  //_selectedName = _fields[0]->getName();
  _selectedLabelWidget = new QLabel(_selectedLabel.c_str(), this);
  QFont font6 = _selectedLabelWidget->font();
  font6.setPixelSize(fsize6);
  _selectedLabelWidget->setFont(font6);
  _fieldsLayout->addWidget(_selectedLabelWidget, row, 0, 1, nCols, alignCenter);

  row++;


  QFont font4 = _selectedLabelWidget->font();
  font4.setPixelSize(fsize4);
  QFont font2 = _selectedLabelWidget->font();
  font2.setPixelSize(fsize2);
  QFont font = _selectedLabelWidget->font();
  font.setPixelSize(fsize);
  */

  QLabel dummy;
  QFont font = dummy.font();
  QFont font2 = dummy.font();
  QFont font4 = dummy.font();

  _valueLabel = new QLabel("", this);
  _valueLabel->setFont(font);
  _fieldsLayout->addWidget(_valueLabel); // , row, 0, 1, nCols, alignCenter);
  //row++;

  QLabel *fieldHeader = new QLabel("FIELDS", this);
  fieldHeader->setFont(font);
  _fieldsLayout->addWidget(fieldHeader); // , row, 0, 1, nCols, alignCenter);
  _fieldsLayout->addStretch();
  //row++;

  //QLabel *nameHeader = new QLabel("Name", this);
  //nameHeader->setFont(font);
  //_fieldsLayout->addWidget(nameHeader); // , row, 0, alignCenter);
  //QLabel *keyHeader = new QLabel("HotKey", this);
  //keyHeader->setFont(font);
  //_fieldsLayout->addWidget(keyHeader, row, 1, alignCenter);
  //if (_haveFilteredFields) {
  //  QLabel *rawHeader = new QLabel("Raw", this);
  //  rawHeader->setFont(font);
  //  _fieldsLayout->addWidget(rawHeader, row, 2, alignCenter);
  //  QLabel *filtHeader = new QLabel("Filt", this);
  //  filtHeader->setFont(font);
  //  _fieldsLayout->addWidget(filtHeader, row, 3, alignCenter);
  //}
  //row++;
  //_rowOffset = row;

/* TODO: fix this up
  // add fields, one row at a time
  // a row can have 1 or 2 buttons, depending on whether the
  // filtered field is present
  size_t nFields = _displayFieldController->getNFields();
  for (size_t ifield = 0; ifield < nFields; ifield++) {

    // get raw field - always present
    
    const DisplayField *rawField = _displayFieldController->getField(ifield); // _fields[ifield];
    int buttonRow = rawField->getButtonRow();
    
    // get filt field - may not be present
    const DisplayField *filtField = _displayFieldController->getFiltered(ifield, buttonRow);

    QLabel *label = new QLabel(this);
    label->setFont(font);
    label->setText(rawField->getLabel().c_str());
    QLabel *key = new QLabel(this);
    key->setFont(font);
    if (rawField->getShortcut().size() > 0) {
      char text[4];
      text[0] = rawField->getShortcut()[0];
      text[1] = '\0';
      key->setText(text);
      char text2[128];
      sprintf(text2, "Hit %s for %s, ALT-%s for filtered",
        text, rawField->getName().c_str(), text);
      label->setToolTip(text2);
      key->setToolTip(text2);
    }

    QRadioButton *rawButton = new QRadioButton(this);
    rawButton->setToolTip(rawField->getName().c_str());
    if (ifield == 0) {
      rawButton->click();
    }
    _fieldsLayout->addWidget(label, row, 0, alignCenter);
    _fieldsLayout->addWidget(key, row, 1, alignCenter);
    _fieldsLayout->addWidget(rawButton, row, 2, alignCenter);
    _fieldGroup->addButton(rawButton, ifield);
    // connect slot for field change
    connect(rawButton, SIGNAL(toggled(bool)), this, SLOT(_changeFieldVariable(bool)));

    _fieldButtons.push_back(rawButton);
    if (filtField != NULL) {
      QRadioButton *filtButton = new QRadioButton(this);
      filtButton->setToolTip(filtField->getName().c_str());
      _fieldsLayout->addWidget(filtButton, row, 3, alignCenter);
      _fieldGroup->addButton(filtButton, ifield + 1);
      _fieldButtons.push_back(filtButton);
      // connect slot for field change
      connect(filtButton, SIGNAL(toggled(bool)), this, SLOT(_changeFieldVariable(bool)));
    }

    _displayFieldController->setVisible(ifield);

    if (filtField != NULL) {
      ifield++;
    }
    _fieldsLayout->setRowStretch(row, 1);

    row++;
  }
*/
  

  //QLabel *spacerRow = new QLabel("", this);
  //_fieldsLayout->addWidget(spacerRow, row, 0);
  //_fieldsLayout->setRowStretch(row, 1);
  //row++;

  // HERE <<=== 
  //  _lastButtonRowFixed = row; // Q: is this just the size of _fieldButtons?
  // connect slot for field change
  
  //connect(_fieldGroup, SIGNAL(buttonClicked(int)),
  //        this, SLOT(_changeField(int)));

  LOG(DEBUG) << "exit";
}

/* TODO: fix this ...
void DisplayFieldView::update() {
  // TODO: get the displayfields from the controller

}
*/

//////////////////////////////////////////////
// update the field panel

void DisplayFieldView::updateFieldPanel(string rawFieldLabel, string newFieldName,
  string rawFieldShortCut)
{
  LOG(DEBUG) << "enter";

  //ParamFile *_params = ParamFile::Instance();

  Qt::Alignment alignCenter(Qt::AlignCenter);
  Qt::Alignment alignRight(Qt::AlignRight);
  
  int fsize = _label_font_size;
  int fsize2 = _label_font_size + 2;
  int fsize4 = _label_font_size + 4;
  int fsize6 = _label_font_size + 6;

  //int row; //  = _rowOffset;
  //int nCols = 3;
  //if (_haveFilteredFields) {
  //  nCols = 4;
  //}
  //QFont font = _selectedLabelWidget->font();
  QLabel dummy;
  QFont font = dummy.font();
  font.setPixelSize(fsize);


/* TODO: fix this ...
  // a row can have 1 or 2 buttons, depending on whether the
  // filtered field is present

  size_t nFields = _displayFieldCsontroller->getNFields(); // 0 - based index
  size_t ifield = _displayFieldController->getFieldIndex(newFieldName);
  //if (ifield < lastButtonRowFixed - 1) {
    // field already in panel
  //  return;
  //}

    // get raw field - always present
    */
    // TODO: these should probably be signals ...
 // DisplayField *rawField = _displayFieldController->getField(ifield); //_fields[ifield];
  //if (rawField->isHidden()) {  // TODO: move to controller
  //  int lastButtonRowFixed = _fieldButtons.size(); // 1 - based index
  //  int _rowOffset = 3;
  //  row = lastButtonRowFixed + _rowOffset;
    //rawField->setButtonRow(row);  // TODO: why does the displayField need the button row?
    
    // get filt field - may not be present
    //const DisplayField *filtField = _displayFieldController->getFiltered(ifield, -1);

//----
//    _spolDivColorMapLabel = new ClickableLabel();
//ColorMapTemplates.cc:    // connect(cmapLabel, &ClickableLabel::clicked, this, &ParameterColorDialog::pickColorPalette);
//ColorMapTemplates.cc:    connect(_defaultColorMapLabel,   &ClickableLabel::clicked, this, &ColorMapTemplates::defaultClicked);

//---

    // TODO: get rid of the buttons, and use the clickable label,
    // one click switches to that field for display
    // two clicks brings up editing for the field, color map, etc. 
    //QLabel *label = new QLabel(this);
    ClickableLabelNamed *label = new ClickableLabelNamed(this);
    // TODO: HERE ===> contextMenu is not here .... do we want DisplayField Dialog?
    // make a special dialog for this field only?
    //connect(label, SIGNAL(ClickableLabel::clicked), this, SLOT(contextMenuParameterColors));
    label->setFont(font);
    label->setText(rawFieldLabel.c_str()); // (rawField->getLabel().c_str());
    //QLabel *key = new QLabel(this);
    //key->setFont(font);
    //if (rawFieldShortCut.size() > 0) {
    //  char text[4];
    //  text[0] = rawFieldShortCut[0];
    //  text[1] = '\0';
    //  key->setText(text);
    //  char text2[128];
    //  sprintf(text2, "Hit %s for %s, ALT-%s for filtered",
    //    text, newFieldName.c_str(), text);
    //  label->setToolTip(text2);
    //  key->setToolTip(text2);
    //}

    //QRadioButton *rawButton = new QRadioButton(newFieldName.c_str(), this);
    //rawButton->setToolTip(newFieldName.c_str());
    //if (ifield == 0) {
     // rawButton->click();
    //}
    //row = buttonRow + 4;
    _fieldsLayout->addWidget(label); // , row, 0, alignCenter);
    //_fieldsLayout->addWidget(key, row, 1, alignCenter);
    //_fieldsLayout->addWidget(rawButton); // , row, 2, alignCenter);
    //_fieldGroup->addButton(rawButton,  lastButtonRowFixed);  //  ifield);
    // connect slot for field change
    //connect(rawButton, SIGNAL(toggled(bool)), this, SLOT(_changeFieldVariable(bool)));
    connect(label, SIGNAL(clicked(QString)), this, SLOT(_changeFieldVariable(QString)));
    connect(label, SIGNAL(doubleClicked(QString)), this, SLOT(_editFieldVariable(QString)));  
    //TODO: how to determine which label was clicked???  
    _fieldButtons.push_back(label);
    /* TODO: fix up ...
    if (filtField != NULL) {
      QRadioButton *filtButton = new QRadioButton(this);
      filtButton->setToolTip(filtField->getName().c_str());
      _fieldsLayout->addWidget(filtButton, row, 3, alignCenter);
      _fieldGroup->addButton(filtButton, ifield + 1);
      _fieldButtons.push_back(filtButton);
      // connect slot for field change
      connect(filtButton, SIGNAL(toggled(bool)), this, SLOT(_changeFieldVariable(bool)));
    }
    */

    //if (filtField != NULL) {
    //  ifield++;
    //}

    //QLabel *spacerRow = new QLabel("", this);
    //_fieldsLayout->addWidget(spacerRow, row, 0);
    //_fieldsLayout->setRowStretch(row, 1);
    
   // rawField->setStateVisible();  // TODO: move to controller
   // _displayFieldController->setSelectedField(ifield); // TODO: move to controller

  //}

  // connect slot for field change
  
  //connect(_fieldGroup, SIGNAL(buttonClicked(int)),
  //        this, SLOT(_changeField(int)));

  _fieldsLayout->addStretch();
  LOG(DEBUG) << "exit";
  
}

void DisplayFieldView::_changeField(int fieldIdx) {
  LOG(DEBUG) << "enter fieldIdx= " << fieldIdx;
  LOG(DEBUG) << "exit";
}

void DisplayFieldView::_editFieldVariable(QString fieldName) {

  LOG(DEBUG) << "enter fieldVariable = " << fieldName.toStdString();
  //emit contextMenuParameterColors(fieldName);
  emit ShowParameterColorDialog(fieldName);
  //ShowContextMenu(fieldName);
  LOG(DEBUG) << "exit"; 
}

// TODO: not using this now. Only bring up Parameter Color Editor
void DisplayFieldView::ShowContextMenu(QString fieldName)
{
  _workingWithField = fieldName;

  QMenu contextMenu("Context menu", this);
  
  QAction action1("Cancel", this);
  connect(&action1, SIGNAL(triggered()), this, SLOT(contextMenuCancel()));
  contextMenu.addAction(&action1);

  QAction action3("Change Color Map", this);
  connect(&action3, SIGNAL(triggered()), this, SLOT(contextMenuParameterColors()));
  contextMenu.addAction(&action3);

  QAction action4("Delete", this);
  action4.setToolTip(fieldName);
  connect(&action4, SIGNAL(triggered()), this, SLOT(contextMenuDelete()));
  contextMenu.addAction(&action4);

  QAction action5("Remove", this);

  connect(&action5, SIGNAL(triggered()), this, SLOT(contextMenuRemove()));
  contextMenu.addAction(&action5);
  QAction action6("Undo", this);
  connect(&action6, SIGNAL(triggered()), this, SLOT(contextMenuUndo()));
  contextMenu.addAction(&action6);

  contextMenu.exec(QCursor::pos());
}

void DisplayFieldView::informationMessage()
{
  QMessageBox::information(this, "QMessageBox::information()", "Not implemented");
}

void DisplayFieldView::contextMenuCancel()
{
                                                                                                    
}

void DisplayFieldView::contextMenuParameterColors()
{

  informationMessage();
   
}


// To remove a widget from a layout, call removeWidget(). 
// Calling QWidget::hide() on a widget also effectively removes the widget 
// from the layout until QWidget::show() is called.

void DisplayFieldView::contextMenuDelete()
{

  // TODO: set field to missing value or bad data value
  //emit setFieldToMissing(_workingWithField);
   
}

void DisplayFieldView::contextMenuRemove()
{
  LOG(DEBUG) << "enter" << _workingWithField.toStdString();
  size_t idx = _findFieldIndex(_workingWithField); 
  QLabel *label = _fieldButtons.at(idx);
  _fieldButtons.erase(_fieldButtons.begin() + idx);
  _fieldsLayout->removeWidget(label);

  delete label;
  show();

  //emit removeField(_workingWithField);
  LOG(DEBUG) << "exit";
}

void DisplayFieldView::contextMenuUndo()
{

  informationMessage();
   
}

/*
void DisplayFieldView::_changeFieldVariable(bool value) {

  LOG(DEBUG) << "enter";

  if (value) {
    for (size_t i = 0; i < _fieldButtons.size(); i++) {
      if (_fieldButtons.at(i)->isChecked()) {
        LOG(DEBUG) << "_fieldButton " << i
        << " out of " << _fieldButtons.size() 
        << " is checked";
        QString fieldNameQ = _fieldButtons.at(i)->text();
        string fieldName = fieldNameQ.toStdString();
        //_controller->setSelectedField(fieldName);
        LOG(DEBUG) << "fieldname is " << fieldName;
        emit selectedFieldChanged(fieldNameQ);
      }
    }
    //emit selectedFieldChanged();
  }

  LOG(DEBUG) << "exit";

}
*/

size_t DisplayFieldView::_findFieldIndex(QString fieldName) {
  size_t nFields = _fieldButtons.size();
  size_t i = 0;
  bool found = false;
  while (i<nFields && !found) {
    if (_fieldButtons.at(i)->text().compare(fieldName) != 0) {
      i += 1;
    } else {
      found = true;
    }
  }
  if (found)
    return i;
  else 
    throw std::invalid_argument(fieldName.toStdString());
}

void DisplayFieldView::_changeFieldVariable(QString fieldName) {

  LOG(DEBUG) << "enter";

        //string fieldName = fieldNameQ.toStdString();

        LOG(DEBUG) << "fieldname is " << fieldName.toStdString();
        emit selectedFieldChanged(fieldName);


  LOG(DEBUG) << "exit";

}

void DisplayFieldView::clear() {

  LOG(DEBUG) << "enter";

    //ClickableLabel *label = new ClickableLabel(this);
    // TODO: HERE ===> contextMenu is not here .... do we want DisplayField Dialog?
    // make a special dialog for this field only?
    //connect(label, SIGNAL(ClickableLabel::clicked), this, SLOT(contextMenuParameterColors));
    //label->setFont(font);
    //label->setText(rawFieldLabel.c_str()); // (rawField->getLabel().c_str());
    //QLabel *key = new QLabel(this);
    //key->setFont(font);


    //QRadioButton *rawButton = new QRadioButton(newFieldName.c_str(), this);
    //_fieldsLayout->addWidget(label, row, 0, alignCenter);
    //_fieldsLayout->addWidget(key, row, 1, alignCenter);
    //_fieldsLayout->addWidget(rawButton, row, 2, alignCenter);
    //_fieldGroup->addButton(rawButton,  lastButtonRowFixed);  //  ifield);
    // connect slot for field change
    //connect(rawButton, SIGNAL(toggled(bool)), this, SLOT(_changeFieldVariable(bool)));
/*
    for (vector<QRadioButton> *::iterator it=_fieldButtons.begin();
      it != _fieldButtons.end(); ++it) {
      _fieldsLayout->removeWidget(*it);
    }
    _fieldButtons.clear(); // push_back(rawButton);
*/
    
  LOG(DEBUG) << "exit";
  
}

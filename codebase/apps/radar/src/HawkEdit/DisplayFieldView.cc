#include "DisplayFieldView.hh"
#include <toolsa/LogStream.hh>


DisplayFieldView::DisplayFieldView(DisplayFieldController *displayFieldController) {
  _controller = displayFieldController;
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
  _fieldsLayout = new QGridLayout(this);
  _fieldsLayout->setVerticalSpacing(5);

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
  _fieldsLayout->addWidget(_valueLabel, row, 0, 1, nCols, alignCenter);
  row++;

  QLabel *fieldHeader = new QLabel("FIELD LIST", this);
  fieldHeader->setFont(font);
  _fieldsLayout->addWidget(fieldHeader, row, 0, 1, nCols, alignCenter);
  row++;

  QLabel *nameHeader = new QLabel("Name", this);
  nameHeader->setFont(font);
  _fieldsLayout->addWidget(nameHeader, row, 0, alignCenter);
  QLabel *keyHeader = new QLabel("HotKey", this);
  keyHeader->setFont(font);
  _fieldsLayout->addWidget(keyHeader, row, 1, alignCenter);
  if (_haveFilteredFields) {
    QLabel *rawHeader = new QLabel("Raw", this);
    rawHeader->setFont(font);
    _fieldsLayout->addWidget(rawHeader, row, 2, alignCenter);
    QLabel *filtHeader = new QLabel("Filt", this);
    filtHeader->setFont(font);
    _fieldsLayout->addWidget(filtHeader, row, 3, alignCenter);
  }
  row++;
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

  int row; //  = _rowOffset;
  int nCols = 3;
  if (_haveFilteredFields) {
    nCols = 4;
  }
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
    int lastButtonRowFixed = _fieldButtons.size(); // 1 - based index
    int _rowOffset = 3;
    row = lastButtonRowFixed + _rowOffset;
    //rawField->setButtonRow(row);  // TODO: why does the displayField need the button row?
    
    // get filt field - may not be present
    //const DisplayField *filtField = _displayFieldController->getFiltered(ifield, -1);

//----
//    _spolDivColorMapLabel = new ClickableLabel();
//ColorMapTemplates.cc:    // connect(cmapLabel, &ClickableLabel::clicked, this, &ParameterColorDialog::pickColorPalette);
//ColorMapTemplates.cc:    connect(_defaultColorMapLabel,   &ClickableLabel::clicked, this, &ColorMapTemplates::defaultClicked);

//---

    //QLabel *label = new QLabel(this);
    ClickableLabel *label = new ClickableLabel(this);
    // TODO: HERE ===> contextMenu is not here .... do we want DisplayField Dialog?
    // make a special dialog for this field only?
    //connect(label, SIGNAL(ClickableLabel::clicked), this, SLOT(contextMenuParameterColors));
    label->setFont(font);
    label->setText(rawFieldLabel.c_str()); // (rawField->getLabel().c_str());
    QLabel *key = new QLabel(this);
    key->setFont(font);
    if (rawFieldShortCut.size() > 0) {
      char text[4];
      text[0] = rawFieldShortCut[0];
      text[1] = '\0';
      key->setText(text);
      char text2[128];
      sprintf(text2, "Hit %s for %s, ALT-%s for filtered",
        text, newFieldName.c_str(), text);
      label->setToolTip(text2);
      key->setToolTip(text2);
    }

    QRadioButton *rawButton = new QRadioButton(this);
    rawButton->setToolTip(newFieldName.c_str());
    //if (ifield == 0) {
      rawButton->click();
    //}
    //row = buttonRow + 4;
    _fieldsLayout->addWidget(label, row, 0, alignCenter);
    _fieldsLayout->addWidget(key, row, 1, alignCenter);
    _fieldsLayout->addWidget(rawButton, row, 2, alignCenter);
    _fieldGroup->addButton(rawButton,  lastButtonRowFixed);  //  ifield);
    // connect slot for field change
    connect(rawButton, SIGNAL(toggled(bool)), this, SLOT(_changeFieldVariable(bool)));

    _fieldButtons.push_back(rawButton);
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
    _fieldsLayout->setRowStretch(row, 1);
    
   // rawField->setStateVisible();  // TODO: move to controller
   // _displayFieldController->setSelectedField(ifield); // TODO: move to controller

  //}

  // connect slot for field change
  
  //connect(_fieldGroup, SIGNAL(buttonClicked(int)),
  //        this, SLOT(_changeField(int)));
  LOG(DEBUG) << "exit";
  
}

void DisplayFieldView::_changeFieldVariable(bool value) {

  LOG(DEBUG) << "enter";

  if (value) {
    for (size_t i = 0; i < _fieldButtons.size(); i++) {
      if (_fieldButtons.at(i)->isChecked()) {
        LOG(DEBUG) << "_fieldButton " << i
        << "out of " << _fieldButtons.size() 
        << " is checked";
        QString fieldNameQ = _fieldButtons.at(i)->text();
        string fieldName = fieldNameQ.toStdString();
        _controller->setSelectedField(fieldName);
        LOG(DEBUG) << "fieldname is " << fieldName;
        emit selectedFieldChanged(fieldNameQ);
      }
    }
    //emit selectedFieldChanged();
  }

  LOG(DEBUG) << "exit";

}

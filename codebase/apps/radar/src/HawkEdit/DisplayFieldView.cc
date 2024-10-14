#include "DisplayFieldView.hh"
#include <toolsa/LogStream.hh>

#include <QMenu>
#include <QMessageBox>

DisplayFieldView::DisplayFieldView() { 

  _haveFilteredFields = false;
  _label_font_size = 12;

}

DisplayFieldView::~DisplayFieldView() {

}

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

  QLabel dummy;
  QFont font = dummy.font();
  QFont font2 = dummy.font();
  QFont font4 = dummy.font();

  _valueLabel = new QLabel("", this);
  _valueLabel->setFont(font);
  _fieldsLayout->addWidget(_valueLabel); // , row, 0, 1, nCols, alignCenter);
  //row++;

  //QLabel *fieldHeader = new QLabel("FIELDS", this);
  //fieldHeader->setFont(font);
  //_fieldsLayout->addWidget(fieldHeader); // , row, 0, 1, nCols, alignCenter);
  //_fieldsLayout->insertStretch(-1, 100);

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

  QLabel dummy;
  QFont font = dummy.font();
  font.setPixelSize(fsize);

    ClickableLabelNamed *label = new ClickableLabelNamed(this);
    // TODO: HERE ===> contextMenu is not here .... do we want DisplayField Dialog?
    // make a special dialog for this field only?
    //connect(label, SIGNAL(ClickableLabel::clicked), this, SLOT(contextMenuParameterColors));
    label->setFont(font);
    label->setText(rawFieldLabel.c_str()); // (rawField->getLabel().c_str());

    _fieldsLayout->addWidget(label); // , row, 0, alignCenter);
    // connect slot for field change
    connect(label, SIGNAL(clicked(QString)), this, SLOT(_changeFieldVariable(QString)));
    connect(label, SIGNAL(doubleClicked(QString)), this, SLOT(_editFieldVariable(QString)));  
    //TODO: how to determine which label was clicked???  
    _fieldButtons.push_back(label);

  //_fieldsLayout->addStretch();
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
  _fieldsLayout->takeAt((int) idx);
  // _fieldsLayout->removeWidget(label);  

  delete label;
  show();

  //emit removeField(_workingWithField);
  LOG(DEBUG) << "exit";
}

void DisplayFieldView::removeField(string fieldName)
{
  //LOG(DEBUG) << "enter" << _workingWithField.toStdString();
  size_t idx = _findFieldIndex(QString(fieldName.c_str())); 
  QLabel *label = _fieldButtons.at(idx);
  _fieldButtons.erase(_fieldButtons.begin() + idx);
  _fieldsLayout->takeAt((int) idx);
  // TODO: need to adjust the position of the labels to remove gaps ...

  delete label;
  show();

  //emit removeField(_workingWithField);
  LOG(DEBUG) << "exit";
}

void DisplayFieldView::contextMenuUndo()
{

  informationMessage();
   
}

bool DisplayFieldView::hasField(string fieldName) {
  try {
    size_t idx = _findFieldIndex(QString(fieldName.c_str()));
    if (idx >=0) return true;
    else return false;
  } catch (std::invalid_argument &ex) {
    return false;
  }
}

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
    
  LOG(DEBUG) << "exit";
  
}

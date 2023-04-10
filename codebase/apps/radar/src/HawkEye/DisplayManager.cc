// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
// ** Copyright UCAR (c) 1990 - 2016                                         
// ** University Corporation for Atmospheric Research (UCAR)                 
// ** National Center for Atmospheric Research (NCAR)                        
// ** Boulder, Colorado, USA                                                 
// ** BSD licence applies - redistribution and use in source and binary      
// ** forms, with or without modification, are permitted provided that       
// ** the following conditions are met:                                      
// ** 1) If the software is modified to produce derivative works,            
// ** such modified software should be clearly marked, so as not             
// ** to confuse it with the version available from UCAR.                    
// ** 2) Redistributions of source code must retain the above copyright      
// ** notice, this list of conditions and the following disclaimer.          
// ** 3) Redistributions in binary form must reproduce the above copyright   
// ** notice, this list of conditions and the following disclaimer in the    
// ** documentation and/or other materials provided with the distribution.   
// ** 4) Neither the name of UCAR nor the names of its contributors,         
// ** if any, may be used to endorse or promote products derived from        
// ** this software without specific prior written permission.               
// ** DISCLAIMER: THIS SOFTWARE IS PROVIDED "AS IS" AND WITHOUT ANY EXPRESS  
// ** OR IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED      
// ** WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.    
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
///////////////////////////////////////////////////////////////
// DisplayManager.cc
//
// DisplayManager object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Dec 2014
//
///////////////////////////////////////////////////////////////
//
// Virtual base class for BscanManager and PolarManager
//
///////////////////////////////////////////////////////////////

#include "DisplayManager.hh"
#include "DisplayField.hh"
#include "DisplayElevation.hh"
#include <qtplot/ColorMap.hh>
#include "Params.hh"
#include "Reader.hh"

#include <string>
#include <cmath>
#include <iostream>
#include <QActionGroup>
#include <QApplication>
#include <QButtonGroup>
#include <QFont>
#include <QFormLayout>
#include <QFrame>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QPushButton>
#include <QRadioButton>
#include <QScrollBar>
#include <QAbstractSlider>
#include <QStatusBar>
#include <QVBoxLayout>
#include <QLineEdit>
#include <QCheckBox>
#include <QErrorMessage>
#include <QDateTime>
#include <QDateTimeEdit>
#include <QComboBox>
#include <QRect>
#include <QKeyEvent>

#include <toolsa/toolsa_macros.h>
#include <toolsa/DateTime.hh>
#include <toolsa/LogStream.hh>
#include <toolsa/TaXml.hh>
#include <Radx/RadxFile.hh>

using namespace std;
int DisplayManager::_timerEventCount = 0;

// Constructor

DisplayManager::DisplayManager(const Params &params,
                               Reader *reader,
                               const vector<DisplayField *> &fields,
                               bool haveFilteredFields) :
        QMainWindow(NULL),
        _params(params),
        _reader(reader),
        _initialRay(true),
        _fields(fields),
        _haveFilteredFields(haveFilteredFields)
        
{

  _beamTimerId = 0;
  _frozen = false;
  _fieldNum = 0;
  _prevFieldNum = -1;

  _radarLat = -9999.0;
  _radarLon = -9999.0;
  _radarAltKm = -9999.0;

  _prevAltKm = 0.0;
  _altRateMps = 0.0;

  _altitudeInFeet = false;

  _clickPointFmq.setUrl(_params.click_point_fmq_url);
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    _clickPointFmq.setVerbose();
  } else if (_params.debug) {
    _clickPointFmq.setDebug();
  }

}

// destructor

DisplayManager::~DisplayManager()

{

}

//////////////////////////////////////////////
// create the status panel

void DisplayManager::_createStatusPanel()
{
 
  // Qt::Alignment alignLeft(Qt::AlignLeft);
  // Qt::Alignment alignRight(Qt::AlignRight);
  Qt::Alignment alignCenter(Qt::AlignCenter);
  // Qt::Alignment alignTop(Qt::AlignTop);

  // status panel - rows of label value pairs
  
  _statusPanel = new QGroupBox(_main);
  _statusLayout = new QGridLayout(_statusPanel);
  _statusLayout->setVerticalSpacing(5);

  int row = 0;
  
  // fonts
  
  QLabel dummy;
  QFont font = dummy.font();
  QFont font2 = dummy.font();
  QFont font6 = dummy.font();
  int fsize = _params.label_font_size;
  int fsize2 = _params.label_font_size + 2;
  int fsize6 = _params.label_font_size + 6;
  font.setPixelSize(fsize);
  font2.setPixelSize(fsize2);
  font6.setPixelSize(fsize6);

  // radar and site name
  
  _radarName = new QLabel(_statusPanel);
  string rname(_params.radar_name);
  if (_params.display_site_name) {
    rname += ":";
    rname += _params.site_name;
  }
  _radarName->setText(rname.c_str());
  _radarName->setFont(font6);
  _statusLayout->addWidget(_radarName, row, 0, 1, 4, alignCenter);
  row++;

  // date and time

  _dateVal = new QLabel("9999/99/99", _statusPanel);
  _dateVal->setFont(font2);
  _statusLayout->addWidget(_dateVal, row, 0, 1, 2, alignCenter);
  row++;

  _timeVal = new QLabel("99:99:99.999", _statusPanel);
  _timeVal->setFont(font2);
  _statusLayout->addWidget(_timeVal, row, 0, 1, 2, alignCenter);
  row++;


  // other labels.  Note that we set the minimum size of the column
  // containing the right hand labels in timerEvent() to prevent the
  // wiggling we were seeing in certain circumstances.  For this to work,
  // the default values for these fields must represent the maximum digits
  // posible for each field.

  _elevVal = _createStatusVal("Elev", "-99.99", row++, fsize2);
  _azVal = _createStatusVal("Az", "-999.99", row++, fsize2);

  if (_params.show_status_in_gui.fixed_angle) {
    _fixedAngVal = _createStatusVal("Fixed ang", "-99.99", row++, fsize2);
  } else {
    _fixedAngVal = NULL;
  }
  
  if (_params.show_status_in_gui.volume_number) {
    _volNumVal = _createStatusVal("Volume", "0", row++, fsize);
  } else {
    _volNumVal = NULL;
  }
  
  if (_params.show_status_in_gui.sweep_number) {
    _sweepNumVal = _createStatusVal("Sweep", "0", row++, fsize);
  } else {
    _sweepNumVal = NULL;
  }

  if (_params.show_status_in_gui.n_samples) {
    _nSamplesVal = _createStatusVal("N samp", "0", row++, fsize);
  } else {
    _nSamplesVal = NULL;
  }

  if (_params.show_status_in_gui.n_gates) {
    _nGatesVal = _createStatusVal("N gates", "0", row++, fsize);
  } else {
    _nGatesVal = NULL;
  }

  if (_params.show_status_in_gui.gate_length) {
    _gateSpacingVal = _createStatusVal("Gate len", "0", row++, fsize);
  } else {
    _gateSpacingVal = NULL;
  }
  
  if (_params.show_status_in_gui.pulse_width) {
    _pulseWidthVal = _createStatusVal("Pulse width", "-9999", row++, fsize);
  } else {
    _pulseWidthVal = NULL;
  }

  if (_params.show_status_in_gui.prf_mode) {
    _prfModeVal = _createStatusVal("PRF mode", "Fixed", row++, fsize);
  } else {
    _prfModeVal = NULL;
  }

  if (_params.show_status_in_gui.prf) {
    _prfVal = _createStatusVal("PRF", "-9999", row++, fsize);
  } else {
    _prfVal = NULL;
  }

  if (_params.show_status_in_gui.nyquist) {
    _nyquistVal = _createStatusVal("Nyquist", "-9999", row++, fsize);
  } else {
    _nyquistVal = NULL;
  }

  if (_params.show_status_in_gui.max_range) {
    _maxRangeVal = _createStatusVal("Max range", "-9999", row++, fsize);
  } else {
    _maxRangeVal = NULL;
  }

  if (_params.show_status_in_gui.unambiguous_range) {
    _unambigRangeVal = _createStatusVal("U-A range", "-9999", row++, fsize);
  } else {
    _unambigRangeVal = NULL;
  }

  if (_params.show_status_in_gui.measured_power_h) {
    _powerHVal = _createStatusVal("Power H", "-9999", row++, fsize);
  } else {
    _powerHVal = NULL;
  }

  if (_params.show_status_in_gui.measured_power_v) {
    _powerVVal = _createStatusVal("Power V", "-9999", row++, fsize);
  } else {
    _powerVVal = NULL;
  }

  if (_params.show_status_in_gui.scan_name) {
    _scanNameVal = _createStatusVal("Scan name", "unknown", row++, fsize);
  } else {
    _scanNameVal = NULL;
  }

  if (_params.show_status_in_gui.scan_mode) {
    _sweepModeVal = _createStatusVal("Scan mode", "SUR", row++, fsize);
  } else {
    _sweepModeVal = NULL;
  }

  if (_params.show_status_in_gui.polarization_mode) {
    _polModeVal = _createStatusVal("Pol mode", "Single", row++, fsize);
  } else {
    _polModeVal = NULL;
  }

  if (_params.show_status_in_gui.latitude) {
    _latVal = _createStatusVal("Lat", "-99.999", row++, fsize);
  } else {
    _latVal = NULL;
  }

  if (_params.show_status_in_gui.longitude) {
    _lonVal = _createStatusVal("Lon", "-999.999", row++, fsize);
  } else {
    _lonVal = NULL;
  }

  if (_params.show_status_in_gui.altitude) {
    if (_altitudeInFeet) {
      _altVal = _createStatusVal("Alt(kft)", "-999.999",
                                 row++, fsize, &_altLabel);
    } else {
      _altVal = _createStatusVal("Alt(km)", "-999.999",
                                 row++, fsize, &_altLabel);
    }
  } else {
    _altVal = NULL;
  }

  if (_params.show_status_in_gui.altitude_rate) {
    if (_altitudeInFeet) {
      _altRateVal = _createStatusVal("AltRate(ft/s)", "-999.999",
                                     row++, fsize, &_altRateLabel);
    } else {
      _altRateVal = _createStatusVal("AltRate(m/s)", "-999.999",
                                     row++, fsize, &_altRateLabel);
    }
  } else {
    _altRateVal = NULL;
  }

  if (_params.show_status_in_gui.speed) {
    _speedVal = _createStatusVal("Speed(m/s)", "-999.99", row++, fsize);
  } else {
    _speedVal = NULL;
  }

  if (_params.show_status_in_gui.heading) {
    _headingVal = _createStatusVal("Heading(deg)", "-999.99", row++, fsize);
  } else {
    _headingVal = NULL;
  }

  if (_params.show_status_in_gui.track) {
    _trackVal = _createStatusVal("Track(deg)", "-999.99", row++, fsize);
  } else {
    _trackVal = NULL;
  }

  if (_params.show_status_in_gui.sun_elevation) {
    _sunElVal = _createStatusVal("Sun el (deg)", "-999.999", row++, fsize);
  } else {
    _sunElVal = NULL;
  }

  if (_params.show_status_in_gui.sun_azimuth) {
    _sunAzVal = _createStatusVal("Sun az (deg)", "-999.999", row++, fsize);
  } else {
    _sunAzVal = NULL;
  }

  QLabel *spacerRow = new QLabel("", _statusPanel);
  _statusLayout->addWidget(spacerRow, row, 0);
  _statusLayout->setRowStretch(row, 1);
  row++;

}

//////////////////////////////////////////////
// create the field panel

void DisplayManager::_createFieldPanel()
{
  
  Qt::Alignment alignCenter(Qt::AlignCenter);
  // Qt::Alignment alignRight(Qt::AlignRight);
  
  int fsize = _params.label_font_size;
  int fsize2 = _params.label_font_size + 2;
  int fsize4 = _params.label_font_size + 4;
  int fsize6 = _params.label_font_size + 6;

  _fieldPanel = new QGroupBox(_main);
  _fieldGroup = new QButtonGroup;
  _fieldsLayout = new QGridLayout(_fieldPanel);
  _fieldsLayout->setVerticalSpacing(5);

  int row = 0;
  int nCols = 3;
  if (_haveFilteredFields) {
    nCols = 4;
  }

  _selectedField = _fields[0];
  _selectedLabel = _fields[0]->getLabel();
  _selectedName = _fields[0]->getName();
  _selectedLabelWidget = new QLabel(_selectedLabel.c_str(), _fieldPanel);
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

  _valueLabel = new QLabel("", _fieldPanel);
  _valueLabel->setFont(font);
  _fieldsLayout->addWidget(_valueLabel, row, 0, 1, nCols, alignCenter);
  row++;

  QLabel *fieldHeader = new QLabel("FIELD LIST", _fieldPanel);
  fieldHeader->setFont(font);
  _fieldsLayout->addWidget(fieldHeader, row, 0, 1, nCols, alignCenter);
  row++;

  QLabel *nameHeader = new QLabel("Name", _fieldPanel);
  nameHeader->setFont(font);
  _fieldsLayout->addWidget(nameHeader, row, 0, alignCenter);
  QLabel *keyHeader = new QLabel("HotKey", _fieldPanel);
  keyHeader->setFont(font);
  _fieldsLayout->addWidget(keyHeader, row, 1, alignCenter);
  if (_haveFilteredFields) {
    QLabel *rawHeader = new QLabel("Raw", _fieldPanel);
    rawHeader->setFont(font);
    _fieldsLayout->addWidget(rawHeader, row, 2, alignCenter);
    QLabel *filtHeader = new QLabel("Filt", _fieldPanel);
    filtHeader->setFont(font);
    _fieldsLayout->addWidget(filtHeader, row, 3, alignCenter);
  }
  row++;

  // add fields, one row at a time
  // a row can have 1 or 2 buttons, depending on whether the
  // filtered field is present

  for (size_t ifield = 0; ifield < _fields.size(); ifield++) {

    // get raw field - always present
    
    const DisplayField *rawField = _fields[ifield];
    int buttonRow = rawField->getButtonRow();
    
    // get filt field - may not be present
    
    const DisplayField *filtField = NULL;
    if (ifield < _fields.size() - 1) {
      if (_fields[ifield+1]->getButtonRow() == buttonRow &&
          _fields[ifield+1]->getIsFilt()) {
        filtField = _fields[ifield+1];
      }
    }

    QLabel *label = new QLabel(_fieldPanel);
    label->setFont(font);
    label->setText(rawField->getLabel().c_str());
    QLabel *key = new QLabel(_fieldPanel);
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

    QRadioButton *rawButton = new QRadioButton(_fieldPanel);
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
      QRadioButton *filtButton = new QRadioButton(_fieldPanel);
      filtButton->setToolTip(filtField->getName().c_str());
      _fieldsLayout->addWidget(filtButton, row, 3, alignCenter);
      _fieldGroup->addButton(filtButton, ifield + 1);
      _fieldButtons.push_back(filtButton);
      // connect slot for field change
      connect(filtButton, SIGNAL(toggled(bool)), this, SLOT(_changeFieldVariable(bool)));
    }

    if (filtField != NULL) {
      ifield++;
    }

    row++;
  }

  QLabel *spacerRow = new QLabel("", _fieldPanel);
  _fieldsLayout->addWidget(spacerRow, row, 0);
  _fieldsLayout->setRowStretch(row, 1);
  row++;

  // connect slot for field change
  
  //connect(_fieldGroup, SIGNAL(buttonClicked(int)),
  //        this, SLOT(_changeField(int)));

}

void DisplayManager::_changeFieldVariable(bool value) {

  if (_params.debug) {
    cerr << "DisplayManager:: the field variable was changed ";
    cerr << endl;
  }
  if (value) {
    for (size_t i = 0; i < _fieldButtons.size(); i++) {
      if (_fieldButtons.at(i)->isChecked()) {
        if (_params.debug) cout << "_fieldButton " << i << " is checked" << endl;
 	_changeField(i, true);
      }
    }
  }

}

void DisplayManager::colorMapRedefineReceived(string fieldName, ColorMap newColorMap) {

  LOG(DEBUG_VERBOSE) << "enter"; 
  
  // connect the new color map with the field
  // find the fieldName in the list of FieldDisplays
  bool found = false;
  vector<DisplayField *>::iterator it;
  int fieldId = 0;

  it = _fields.begin(); 
  while ( it != _fields.end() && !found ) {
    DisplayField *field = *it;
   
    string name = field->getName();
    if (name.compare(fieldName) == 0) {
      found = true;
      field->replaceColorMap(newColorMap);
    }
    fieldId++;
    it++;
  }
  if (!found) {
    LOG(ERROR) << fieldName;
    LOG(ERROR) << "ERROR - field not found; no color map change";
    // TODO: present error message box 
  } else {
    // look up the fieldId from the fieldName
    // change the field variable
    _changeField(fieldId, true); 
  }
  
  LOG(DEBUG_VERBOSE) << "exit";
}


void DisplayManager::_openFile() {
}

void DisplayManager::_saveFile() {
}

///////////////////////////////////////////////////////
// create the click report dialog
//
// This shows the field values at the latest click point

void DisplayManager::_createClickReportDialog()
{
  
  _clickReportDialog = new QDialog(this);
  _clickReportDialog->setMinimumSize(100, 100);
  _clickReportDialog->setWindowTitle("Field values");

  Qt::Alignment alignCenter(Qt::AlignCenter);
  // Qt::Alignment alignRight(Qt::AlignRight);
  
  _clickReportDialogLayout = new QGridLayout(_clickReportDialog);
  _clickReportDialogLayout->setVerticalSpacing(5);

  int row = 0;
  QLabel *mainHeader = new QLabel("CLICK POINT DATA", _clickReportDialog);
  _clickReportDialogLayout->addWidget(mainHeader, row, 0, 1, 3, alignCenter);
  row++;

  // _clickReportDialogLayout->addWidget(left, row, 0, alignRight);

  _dateClicked = _addLabelRow(_clickReportDialog, _clickReportDialogLayout,
                              "Date", "9999/99/99", row++);
  _timeClicked = _addLabelRow(_clickReportDialog, _clickReportDialogLayout,
                              "Time", "99:99:99.999", row++);
  _elevClicked = _addLabelRow(_clickReportDialog, _clickReportDialogLayout,
                              "Elevation", "-9999", row++);
  _azClicked = _addLabelRow(_clickReportDialog, _clickReportDialogLayout,
                            "Azimuth", "-9999", row++);
  _gateNumClicked = _addLabelRow(_clickReportDialog, _clickReportDialogLayout,
                                 "Gate num", "-9999", row++);
  _rangeClicked = _addLabelRow(_clickReportDialog, _clickReportDialogLayout,
                               "Range", "-9999", row++);
  _altitudeClicked = _addLabelRow(_clickReportDialog, _clickReportDialogLayout,
                                  "Altitude", "-9999", row++);

  QLabel *valHeader = new QLabel("FIELD VALUES", _clickReportDialog);
  _clickReportDialogLayout->addWidget(valHeader, row, 0, 1, 3, alignCenter);
  row++;

  QLabel *nameHeader = new QLabel("Name", _clickReportDialog);
  _clickReportDialogLayout->addWidget(nameHeader, row, 0, alignCenter);
  QLabel *rawHeader = new QLabel("Raw", _clickReportDialog);
  _clickReportDialogLayout->addWidget(rawHeader, row, 1, alignCenter);
  if (_haveFilteredFields) {
    QLabel *filtHeader = new QLabel("Filt", _clickReportDialog);
    _clickReportDialogLayout->addWidget(filtHeader, row, 2, alignCenter);
  }
  row++;

  // add fields, one row at a time
  // a row can have 1 or 2 buttons, depending on whether the
  // filtered field is present

  for (size_t ifield = 0; ifield < _fields.size(); ifield++) {

    // get raw field - always present
    
    DisplayField *rawField = _fields[ifield];
    int buttonRow = rawField->getButtonRow();
    
    // get filt field - may not be present
    
    DisplayField *filtField = NULL;
    if (ifield < _fields.size() - 1) {
      if (_fields[ifield+1]->getButtonRow() == buttonRow &&
          _fields[ifield+1]->getIsFilt()) {
        filtField = _fields[ifield+1];
      }
    }

    QLabel *label = new QLabel(_clickReportDialog);
    label->setText(rawField->getLabel().c_str());
    _clickReportDialogLayout->addWidget(label, row, 0, alignCenter);
    
    rawField->createDialog(_clickReportDialog, "-------------");
    _clickReportDialogLayout->addWidget(rawField->getDialog(), row, 1, alignCenter);

    if (filtField) {
      filtField->createDialog(_clickReportDialog, "-------------");
      _clickReportDialogLayout->addWidget(filtField->getDialog(), row, 2, alignCenter);
    }
      
    if (filtField != NULL) {
      ifield++;
    }

    row++;
  }
}

//////////////////////////////////////////////
// make a new label with right justification

QLabel *DisplayManager::_newLabelRight(const string &text)
{
  QLabel *label = new QLabel;
  label->setText("-----");
  label->setAlignment(Qt::AlignRight);
  return label;
}

//////////////////////////////////////////////////
// create a row in the status panel

QLabel *DisplayManager::_createStatusVal(const string &leftLabel,
                                         const string &rightLabel,
                                         int row, 
                                         int fontSize,
                                         QLabel **label)
  
{

  QLabel *left = new QLabel(_statusPanel);
  left->setText(leftLabel.c_str());
  Qt::Alignment alignRight(Qt::AlignRight);
  _statusLayout->addWidget(left, row, 0, alignRight);
  if (label != NULL) {
    *label = left;
  }

  QLabel *right = new QLabel(_statusPanel);
  right->setText(rightLabel.c_str());
  // Qt::Alignment alignCenter(Qt::AlignCenter);
  _statusLayout->addWidget(right, row, 1, alignRight);

  if (fontSize > 0) {
    QFont font = left->font();
    font.setPixelSize(fontSize);
    left->setFont(font);
    right->setFont(font);
  }

  _valsRight.push_back(right);

  return right;
}

//////////////////////////////////////////////////
// create a label row in a dialog

QLabel *DisplayManager::_addLabelRow(QWidget *parent,
                                     QGridLayout *layout,
                                     const string &leftLabel,
                                     const string &rightLabel,
                                     int row, 
                                     int fontSize)
  
{

  QLabel *left = new QLabel(parent);
  left->setText(leftLabel.c_str());
  Qt::Alignment alignRight(Qt::AlignRight);
  layout->addWidget(left, row, 0, alignRight);
  
  QLabel *right = new QLabel(parent);
  right->setText(rightLabel.c_str());
  Qt::Alignment alignCenter(Qt::AlignCenter);
  layout->addWidget(right, row, 1, 1, 2, alignCenter);

  if (fontSize > 0) {
    QFont font = left->font();
    font.setPixelSize(fontSize);
    left->setFont(font);
    right->setFont(font);
  }

  return right;
}

//////////////////////////////////////////////////
// create a user input row in a widget

QLineEdit *DisplayManager::_addInputRow(QWidget *parent,
                                        QVBoxLayout *layout,
                                        const string &leftLabel,
                                        const string &rightContent,
                                        int fontSize,
                                        QLabel **label)
  
{
  
  QFrame *frame = new QFrame(parent);
  QHBoxLayout *horiz = new QHBoxLayout;
  frame->setLayout(horiz);
    
  QLabel *left = new QLabel(frame);
  left->setText(leftLabel.c_str());
  horiz->addWidget(left);
  
  QLineEdit *right = new QLineEdit(frame);
  right->setText(rightContent.c_str());
  horiz->addWidget(right);

  layout->addWidget(frame);
  
  if (fontSize > 0) {
    QFont font = left->font();
    font.setPixelSize(fontSize);
    left->setFont(font);
    right->setFont(font);
  }

  if (label != NULL) {
    *label = left;
  }
  return right;

}

//////////////////////////////////////////////////
// create a user input row in a widget

QLineEdit *DisplayManager::_addInputRow(QWidget *parent,
                                        QVBoxLayout *layout,
                                        const string &leftLabel,
                                        const string &rightContent,
                                        int fontSize,
                                        QFrame **framePtr)
  
{
  
  QFrame *frame = new QFrame(parent);
  QHBoxLayout *horiz = new QHBoxLayout;
  frame->setLayout(horiz);
    
  QLabel *left = new QLabel(frame);
  left->setText(leftLabel.c_str());
  horiz->addWidget(left);
  
  QLineEdit *right = new QLineEdit(frame);
  right->setText(rightContent.c_str());
  horiz->addWidget(right);

  layout->addWidget(frame);

  if (fontSize > 0) {
    QFont font = left->font();
    font.setPixelSize(fontSize);
    left->setFont(font);
    right->setFont(font);
  }

  if (framePtr != NULL) {
    *framePtr = frame;
  }
  return right;

}

//////////////////////////////////////////////
// update the status panel

void DisplayManager::_updateStatusPanel(const RadxRay *ray)
{

  // set time etc

  char text[1024];
  
  QString prev_radar_name = _radarName->text();
  
  string rname(_platform.getInstrumentName());
  if (_params.override_radar_name) rname = _params.radar_name;
  if (_params.display_site_name) {
    rname += ":";
    if (_params.override_site_name) {
      rname += _params.site_name;
    } else {
      rname += _platform.getSiteName();
    }
  }
  _radarName->setText(rname.c_str());

  if (prev_radar_name != _radarName->text()) {
    _setTitleBar(rname);
  }
  
  DateTime rayTime(ray->getTimeSecs());
  sprintf(text, "%.4d/%.2d/%.2d",
          rayTime.getYear(), rayTime.getMonth(), rayTime.getDay());
  _dateVal->setText(text);

  sprintf(text, "%.2d:%.2d:%.2d.%.3d",
          rayTime.getHour(), rayTime.getMin(), rayTime.getSec(),
          ((int) ray->getNanoSecs() / 1000000));
  _timeVal->setText(text);
  
  if (_volNumVal) {
    _setText(text, "%d", ray->getVolumeNumber());
    _volNumVal->setText(text);
  }
  
  if (_sweepNumVal) {
    _setText(text, "%d", ray->getSweepNumber());
    _sweepNumVal->setText(text);
  }
  
  if (_fixedAngVal) {  
    _setText(text, "%6.2f", ray->getFixedAngleDeg());
    _fixedAngVal->setText(text);
  }
  
  if (_elevVal) {
    if (fabs(ray->getElevationDeg()) < 1000) {
      _setText(text, "%6.2f", ray->getElevationDeg());
      _elevVal->setText(text);
    }
  }

  if (_azVal) {
    if (fabs(ray->getAzimuthDeg()) < 1000) {
      _setText(text, "%6.2f", ray->getAzimuthDeg());
      _azVal->setText(text);
    }
  }
  
  if (_nSamplesVal) {
    _setText(text, "%d", (int) ray->getNSamples());
    _nSamplesVal->setText(text);
  }
  
  if (_nGatesVal) {
    _setText(text, "%d", (int) ray->getNGates());
    _nGatesVal->setText(text);
  }
  
  if (_gateSpacingVal) {
    _setText(text, "%.4f", ray->getGateSpacingKm());
    _gateSpacingVal->setText(text);
  }
  
  if (_pulseWidthVal) {
    _setText(text, "%.3f", ray->getPulseWidthUsec());
    _pulseWidthVal->setText(text);
  }

  if (_prfVal) {
    if (ray->getPrtMode() == Radx::PRT_MODE_FIXED) {
      if (ray->getPrtSec() <= 0) {
        _setText(text, "%d", -9999);
      } else {
        _setText(text, "%d", (int) ((1.0 / ray->getPrtSec()) * 10.0 + 0.5) / 10);
      }
    } else {
      double prtSec = ray->getPrtSec();
      if (prtSec <= 0) {
        _setText(text, "%d", -9999);
      } else {
        int iprt = (int) ((1.0 / ray->getPrtSec()) * 10.0 + 0.5) / 10;
        double prtRatio = ray->getPrtRatio();
        if (prtRatio > 0.6 && prtRatio < 0.7) {
          _setText(text, "%d(2/3)", iprt);
        } else if (prtRatio < 0.775) {
          _setText(text, "%d(3/4)", iprt);
        } else if (prtRatio < 0.825) {
          _setText(text, "%d(4/5)", iprt);
        } else {
          _setText(text, "%d", iprt);
        }
      }
    }
    _prfVal->setText(text);
  }

  if (_nyquistVal) {
    if (fabs(ray->getNyquistMps()) < 1000) {
      _setText(text, "%.1f", ray->getNyquistMps());
      _nyquistVal->setText(text);
    }
  }

  if (_maxRangeVal) {
    double maxRangeData = ray->getStartRangeKm() +
      ray->getNGates() * ray->getGateSpacingKm();
    _setText(text, "%.1f", maxRangeData);
    _maxRangeVal->setText(text);
  }

  if (_unambigRangeVal) {
    if (fabs(ray->getUnambigRangeKm()) < 100000) {
      _setText(text, "%.1f", ray->getUnambigRangeKm());
      _unambigRangeVal->setText(text);
    }
  }
  
  if (_powerHVal) {
    if (ray->getMeasXmitPowerDbmH() > -9990) {
      _setText(text, "%.1f", ray->getMeasXmitPowerDbmH());
      _powerHVal->setText(text);
    }
  }
   
  if (_powerVVal) {
    if (ray->getMeasXmitPowerDbmV() > -9990) {
      _setText(text, "%.1f", ray->getMeasXmitPowerDbmV());
      _powerVVal->setText(text);
    }
  }

  if (_scanNameVal) {
    string scanName = ray->getScanName();
    size_t len = scanName.size();
    if (len > 8) {
      scanName = scanName.substr(0, 8);
    }
    _scanNameVal->setText(scanName.c_str());
  }

  if (_sweepModeVal) {
    switch (ray->getSweepMode()) {
      case Radx::SWEEP_MODE_SECTOR: {
        _sweepModeVal->setText("sector"); break;
      }
      case Radx::SWEEP_MODE_COPLANE: {
        _sweepModeVal->setText("coplane"); break;
      }
      case Radx::SWEEP_MODE_RHI: {
        _sweepModeVal->setText("RHI"); break;
      }
      case Radx::SWEEP_MODE_VERTICAL_POINTING: {
        _sweepModeVal->setText("vert"); break;
      }
      case Radx::SWEEP_MODE_IDLE: {
        _sweepModeVal->setText("idle"); break;
      }
      case Radx::SWEEP_MODE_AZIMUTH_SURVEILLANCE:
      case Radx::SWEEP_MODE_ELEVATION_SURVEILLANCE: {
        _sweepModeVal->setText("SUR"); break;
      }
      case Radx::SWEEP_MODE_SUNSCAN: {
        _sweepModeVal->setText("sunscan"); break;
      }
      case Radx::SWEEP_MODE_SUNSCAN_RHI: {
        _sweepModeVal->setText("sun_rhi"); break;
      }
      case Radx::SWEEP_MODE_POINTING: {
        _sweepModeVal->setText("point"); break;
      }
      case Radx::SWEEP_MODE_CALIBRATION: {
        _sweepModeVal->setText("cal"); break;
      }
      default: {
        _sweepModeVal->setText("unknown");
      }
    }
  }

  if (_polModeVal) {
    _polModeVal->setText(Radx::polarizationModeToStr
                         (ray->getPolarizationMode()).c_str());
  }
   
  if (_prfModeVal) {
    _prfModeVal->setText(Radx::prtModeToStr
                         (ray->getPrtMode()).c_str());
  }

  if (fabs(_radarLat - _platform.getLatitudeDeg()) > 0.0001 ||
      fabs(_radarLon - _platform.getLongitudeDeg()) > 0.0001 ||
      fabs(_radarAltKm - _platform.getAltitudeKm()) > 0.0001) {
    _radarLat = _platform.getLatitudeDeg();
    _radarLon = _platform.getLongitudeDeg();
    _radarAltKm = _platform.getAltitudeKm();
    if (_radarAltKm < -1.0) {
      _radarAltKm = 0.0;
    }
    _sunPosn.setLocation(_radarLat, _radarLon, _radarAltKm * 1000.0);
  }

  if (ray->getGeoreference() != NULL) {

    if (_latVal) {
      _radarLat = ray->getGeoreference()->getLatitude();
      _setText(text, "%.3f", _radarLat);
      _latVal->setText(text);
    }
     
    if (_lonVal) {
      _radarLon = ray->getGeoreference()->getLongitude();
      _setText(text, "%.3f", _radarLon);
      _lonVal->setText(text);
    }

    if (_altVal) {
      _radarAltKm = ray->getGeoreference()->getAltitudeKmMsl();
      if (_altitudeInFeet) {
        _setText(text, "%.3f", _radarAltKm / 0.3048);
      } else {
        _setText(text, "%.3f", _radarAltKm);
      }
      _altVal->setText(text);
    }

    // compute altitude rate every 2 secs
    
    if (_prevAltKm > -9990) {
      double deltaTime = ray->getRadxTime() - _prevAltTime;
      if (deltaTime > 2.0) {
        double altKm = ray->getGeoreference()->getAltitudeKmMsl();
        double deltaAltKm = altKm - _prevAltKm;
        _altRateMps = (deltaAltKm / deltaTime) * 1000.0;
        _prevAltKm = ray->getGeoreference()->getAltitudeKmMsl();
        _prevAltTime = ray->getRadxTime();
      }
    } else {
      _prevAltKm = ray->getGeoreference()->getAltitudeKmMsl();
      _prevAltTime = ray->getRadxTime();
    }
    
    if (_altitudeInFeet) {
      _setText(text, "%.1f", _altRateMps / 0.3048);
    } else {
      _setText(text, "%.1f", _altRateMps);
    }
    if (_altRateVal) {
      _altRateVal->setText(text);
    }

    if (_speedVal) {
      double ewVel = ray->getGeoreference()->getEwVelocity();
      double nsVel = ray->getGeoreference()->getNsVelocity();
      double speed = sqrt(ewVel * ewVel + nsVel * nsVel);
      _setText(text, "%.2f", speed);
      _speedVal->setText(text);
    }
     
    if (_headingVal) {
      double heading = ray->getGeoreference()->getHeading();
      if (heading >= 0 && heading <= 360.0) {
        _setText(text, "%.2f", heading);
        _headingVal->setText(text);
      }
    }
     
    if (_trackVal) {
      double track = ray->getGeoreference()->getTrack();
      if (track >= 0 && track <= 360.0) {
        _setText(text, "%.2f", track);
        _trackVal->setText(text);
      }
    }
     
  } else {
    
    _setText(text, "%.3f", _platform.getLatitudeDeg());
    if (_latVal) {
      _latVal->setText(text);
    }
    
    _setText(text, "%.3f", _platform.getLongitudeDeg());
    if (_lonVal) {
      _lonVal->setText(text);
    }
    
    if (_altitudeInFeet) {
      if (_platform.getAltitudeKm() > -1.0) {
        _setText(text, "%.3f", _platform.getAltitudeKm() / 0.3048);
      } else {
        snprintf(text, 1024, "%s", "missing");
      }
    } else {
      _setText(text, "%.3f", _platform.getAltitudeKm());
    }
    if (_altVal) {
      _altVal->setText(text);
    }
    if (_altRateVal) {
      _altRateVal->setText("0.0");
    }

  }
  
  double sunEl, sunAz;
  _sunPosn.computePosn(ray->getTimeDouble(), sunEl, sunAz);
  _setText(text, "%.3f", sunEl);
  if (_sunElVal) {
    _sunElVal->setText(text);
  }
  _setText(text, "%.3f", sunAz);
  if (_sunAzVal) {
    _sunAzVal->setText(text);
  }

}

///////////////////////////////////////////
// set text for GUI panels

void DisplayManager::_setText(char *text,
                              const char *format,
                              int val)
{
  if (abs(val) < 9999) {
    sprintf(text, format, val);
  } else {
    sprintf(text, format, -9999);
  }
}

void DisplayManager::_setText(char *text,
                              const char *format,
                              double val)
{
  if (fabs(val) < 9999) {
    sprintf(text, format, val);
  } else {
    sprintf(text, format, -9999.0);
  }
}

////////////////////////////////////////////////////////////////

double DisplayManager::_getInstHtKm(const RadxRay *ray)

{
  double instHtKm = _platform.getAltitudeKm();
  if (instHtKm < -1.0) {
    instHtKm = 0.0;
  }
  if (ray->getGeoreference() != NULL) {
    instHtKm = ray->getGeoreference()->getAltitudeKmMsl();
  }
  return instHtKm;
}

/////////////////////////////////////////////////////////////////////  
// slots

/////////////////////////////
// show data at click point
void DisplayManager::_showClick()
{
  if (_clickReportDialog) {
    if (_clickReportDialog->isVisible()) {
      _clickReportDialog->setVisible(false);
    } else {
      if (_clickReportDialog->x() == 0 &&
          _clickReportDialog->y() == 0) {
        QPoint pos;
        pos.setX(x() + width() + 5);
        pos.setY(y());
        _clickReportDialog->move(pos);
      }
      _clickReportDialog->setVisible(true);
      _clickReportDialog->raise();
    }
  }
}

/////////////////////////////////////////////////////
// howto help

void DisplayManager::_howto()
{
  string text;
  text += "HOWTO HINTS FOR HAWK-EYE\n";
  text += "========================\n";
  text += "\n";
  text += "Use NUMBER keys to display RAW fields\n";
  text += "Use ALT-NUMBER keys to display FILTERED fields\n";
  text += "Hit '.' to toggle between the two latest fields\n";
  text += "\n";
  text += "You can also use the arrow keys to select different fields\n";
  text += "\n";
  text += "Click in main window to get field data at a point.\n";
  QMessageBox::about(this, tr("Howto dialog"), tr(text.c_str()));
}

void DisplayManager::_about()
{
  //QMessageBox::about(this, tr("About Menu"),
		     //tr("HawkEye is an engineering display for beam-by-beam radar data. "));
  string text;
  
  text += "HawkEye is an LROSE application for engineering and research display of radar data. \n\n";
  text += "Get help with HawkEye ...  \n ";
  text += "\nReport an issue https://github.com/NCAR/lrose-core/issues \n ";
  text += "\nHawkEye Version ... \n ";  
  text += "\nCopyright UCAR (c) 1990 - 2019  ";
  text += "\nUniversity Corporation for Atmospheric Research (UCAR)  ";  
  text += "\nNational Center for Atmospheric Research (NCAR)   ";  
  text += "\nBoulder, Colorado, USA ";  
  text += "\n\nBSD licence applies - redistribution and use in source and binary";  
  text += " forms, with or without modification, are permitted provided that";  
  text += " the following conditions are met: ";  
  text += "\n1) If the software is modified to produce derivative works,";  
  text += " such modified software should be clearly marked, so as not";  
  text += " to confuse it with the version available from UCAR. ";  
  text += "\n2) Redistributions of source code must retain the above copyright";  
  text += " notice, this list of conditions and the following disclaimer.";  
  text += "\n3) Redistributions in binary form must reproduce the above copyright";  
  text += " notice, this list of conditions and the following disclaimer in the";  
  text += " documentation and/or other materials provided with the distribution.";  
  text += "\n4) Neither the name of UCAR nor the names of its contributors,";  
  text += "if any, may be used to endorse or promote products derived from";  
  text += " this software without specific prior written permission.";  
  text += "\n\nDISCLAIMER: THIS SOFTWARE IS PROVIDED \"AS IS\" AND WITHOUT ANY EXPRESS ";  
  text += " OR IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED";  
  text += " WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.";  

  QMessageBox::about(this, tr("About Menu"), tr(text.c_str()));
}

/////////////////////////////////////////////////////////////////
// write click point data, in XML format, to FMQ

int DisplayManager::_writeClickPointXml2Fmq(const RadxRay *ray,
                                            double rangeKm,
                                            int gateNum)

{
  
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    fprintf(stderr, "DisplayManager::_writeClickPointXml2Fmq() called\n");
  }

  if (_clickPointFmq.write(ray->getTimeSecs(),
                           ray->getNanoSecs(),
                           ray->getAzimuthDeg(),
                           ray->getElevationDeg(),
                           rangeKm,
                           gateNum)) {
    cerr << "ERROR - DisplayManager::_writeClickPointXml2Fmq()" << endl;
    return -1;
  }

  return 0;

}

/////////////////////////////////////////////////
// read click point data from FMQ
// Returns 0 on success, -1 on failure

int DisplayManager::_readClickPointFmq(bool &gotNew)
  
{
  
  // read in a new message
  
  if (_clickPointFmq.read(gotNew)) {
    cerr << "ERROR -  DisplayManager::_readClickPointFmq" << endl;
    cerr << "  Cannot read click point info from FMQ" << endl;
    cerr << "  Fmq: " << _params.click_point_fmq_url << endl;
    return -1;
  }
  
  if (!gotNew) {
    // no data
    return 0;
  }

  if (_params.debug) {
    cerr << "=========== latest click point XML ==================" << endl;
    cerr << "_clickPointTime: " << _clickPointFmq.getDataTime().asString(6) << endl;
    cerr << "_clickPointElevation: " << _clickPointFmq.getElevation() << endl;
    cerr << "_clickPointAzimuth: " << _clickPointFmq.getAzimuth() << endl;
    cerr << "_clickPointRangeKm: " << _clickPointFmq.getRangeKm() << endl;
    cerr << "_clickPointGateNum: " << _clickPointFmq.getGateNum() << endl;
    cerr << "=====================================================" << endl;
  }
  
  return 0;

}



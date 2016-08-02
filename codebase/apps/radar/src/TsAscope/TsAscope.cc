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
// TsAscope.cc
//
// TsAscope object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Dec 2011
//
///////////////////////////////////////////////////////////////
//
// TsAscope is an AScope for IWRF time series data
//
///////////////////////////////////////////////////////////////

#include "TsAscope.hh"
#include "Params.hh"
#include "Reader.hh"
#include <radar/RadarComplex.hh>

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
#include <QStatusBar>
#include <QVBoxLayout>

#include <rapmath/math_macros.h>
#include <toolsa/DateTime.hh>

using namespace std;
bool TsAscope::_firstTimerEvent = true;

// Constructor

TsAscope::TsAscope(int argc, char **argv, QWidget *parent) :
        QMainWindow(parent),
        _args("TsAscope"),
        _argc(argc),
        _argv(argv),
	_rhiWindowDisplayed(false)
{

  OK = 1;
  _beamTimerId = 0;
  _frozen = false;
  _ppi = NULL;
  _rhi = NULL;
  _reader = NULL;
  _prevAz = -9999.0;
  _prevEl = -9999.0;
  _startAz = -9999.0;
  _endAz = -9999.0;
  _ppiRays = NULL;
  _rhiRays = NULL;
  _fieldNum = 0;
  _prevFieldNum = -1;

  // set programe name

  _progName = strdup("TsAscope");
  
  // get command line args
  
  if (_args.parse(argc, (const char **) argv)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with command line args" << endl;
    OK = 0;
    return;
  }

  // load TDRP params from command line
  
  char *paramsPath = (char *) "unknown";
  if (_params.loadFromArgs(argc, argv,
			   _args.override.list,
			   &paramsPath)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with TDRP parameters." << endl;
    OK = false;
    return;
  }

  if (_params.fields_n < 1) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "  0 fields specified" << endl;
    cerr << "  At least 1 field is required" << endl;
    OK = false;
    return;
  }

  // initialize geometry

  _nGates = 1000;
  _maxRange = _params.max_range_km;

  // set up color maps

  if (_setupColorMaps()) {
    OK = false;
    return;
  }

  // initialize fields

  for (int ifield = 0; ifield < _params.fields_n; ifield++) {
    const Params::field_t &fld = _params._fields[ifield];
    Field field;
    field.name = fld.raw_name;
    field.units = fld.units;
    field.colorMap = _colorMaps[ifield*2];
    _fields.push_back(field);
    field.name = fld.filtered_name;
    field.colorMap = _colorMaps[ifield*2+1];
    _fields.push_back(field);
  }
  _nFields = _fields.size();

  // set up windows

  _setupWindows();

  // create reader

  if (_setupReader()) {
    OK = false;
    return;
  }

  // set up ray locators

  _ppiRays = new RayLoc[RAY_LOC_N];
  _ppiRayLoc = _ppiRays + RAY_LOC_OFFSET;

  _rhiRays = new RayLoc[RAY_LOC_N];
  _rhiRayLoc = _rhiRays + RAY_LOC_OFFSET;

  // set initial field to 0

  _changeField(0);

}

// destructor

TsAscope::~TsAscope()

{

  if (_ppi) {
    delete _ppi;
  }

  if (_rhi) {
    delete _rhi;
  }

  if (_reader) {
    delete _reader;
  }

  if (_ppiRays) {
    delete[] _ppiRays;
  }
  
  if (_rhiRays) {
    delete[] _rhiRays;
  }
  
}

//////////////////////////////////////////////////
// Run

int TsAscope::Run(QApplication &app)
{

  // start the reader thread

  _reader->start();

  // make window visible
  
  show();
  
  // set timer running

  _beamTimerId = startTimer(2);

  return app.exec();

}

//////////////////////////////////////////////////
// set up color maps
// use same map for raw and unfiltered fields
// returns 0 on success, -1 on failure
  
int TsAscope::_setupColorMaps()
{
  
  for (int ifield = 0; ifield < _params.fields_n; ifield++) {
    string colorMapPath = _params.color_scale_dir;
    colorMapPath += PATH_DELIM;
    colorMapPath += _params._fields[ifield].color_map;
    ColorMap *map = new ColorMap();
    if (map->readRalMap(colorMapPath)) {
      return -1;
    }
    map->setName(_params._fields[ifield].label);
    map->setUnits(_params._fields[ifield].units);
    _colorMaps.push_back(map);
    // we add 2 identical color maps since
    // we need one for raw and one for filtered
    ColorMap *copy = new ColorMap(*map);
    _colorMaps.push_back(copy);
  }

  return 0;

}

//////////////////////////////////////////////////
// set up windows and widgets
  
void TsAscope::_setupWindows()
{

  // set up windows

  _main = new QFrame(this);
  setCentralWidget(_main);
  
  // ppi - main window

  _ppiParent = new QFrame(_main);
  _ppiParent->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

  // configure the PPI

  _ppi = new PpiWidget(_ppiParent, _params);
  _ppi->configure(_nFields, _nGates, _params.background_render_mins, _maxRange);
  _ppi->setRings(true);
  _ppi->setGrids(false);
  _ppi->setAzLines(true);

  connect(this, SIGNAL(frameResized(const int, const int)),
	  _ppi, SLOT(resize(const int, const int)));
  
  // info label at bottom - remove later
  //   _infoLabel = new QLabel(tr("<i>Choose a menu option, or right-click to "
  //                              "invoke a context menu</i>"), _main);
  //   _infoLabel->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
  //   _infoLabel->setAlignment(Qt::AlignCenter);

  // Create the RHI window

  _createRhiWindow();
  
  // create status panel

  _createStatusPanel();

  // create fields panel
  
  _createFieldPanel();

  // color bar to right

  _colorBar = new ColorBar(_params.color_scale_width,
                           _colorMaps[0], _main);
  
  // main window layout
  
  QHBoxLayout *mainLayout = new QHBoxLayout(_main);
  mainLayout->setMargin(3);
  mainLayout->addWidget(_statusPanel);
  mainLayout->addWidget(_fieldPanel);
  mainLayout->addWidget(_ppiParent);
  mainLayout->addWidget(_colorBar);
  // _main->setLayout(mainLayout);

  _createActions();
  _createMenus();

  // QString message = tr("A context menu is available by right-clicking");
  // statusBar()->showMessage(message);

  setRadarName(_params.radar_name);
  setMinimumSize(400, 300);
  resize(_params.main_window_width, _params.main_window_height);

  QPoint pos;
  pos.setX(_params.main_window_start_x);
  pos.setY(_params.main_window_start_y);
  move(pos);

  // set up field status dialog

  _createFieldStatusDialog();

}

//////////////////////////////////////////////
// create the status panel

void TsAscope::_createStatusPanel()
{
 
  Qt::Alignment alignLeft(Qt::AlignLeft);
  Qt::Alignment alignRight(Qt::AlignRight);
  Qt::Alignment alignCenter(Qt::AlignCenter);
  Qt::Alignment alignTop(Qt::AlignTop);

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

  _dateLabel = new QLabel("9999/99/99", _statusPanel);
  _dateLabel->setFont(font2);
  _statusLayout->addWidget(_dateLabel, row, 0, 1, 2, alignCenter);
  row++;

  _timeLabel = new QLabel("99:99:99.999", _statusPanel);
  _timeLabel->setFont(font2);
  _statusLayout->addWidget(_timeLabel, row, 0, 1, 2, alignCenter);
  row++;


  // other labels.  Note that we set the minimum size of the column
  // containing the right hand labels in timerEvent() to prevent the
  // wiggling we were seeing in certain circumstances.  For this to work,
  // the default values for these fields must represent the maximum digits
  // posible for each field.

  _fixedAngLabel = _createStatusLabel("Fixed ang", "-99.99", row++, fsize2);
  _elevLabel = _createStatusLabel("Elevation", "-99.99", row++, fsize2);
  _azLabel = _createStatusLabel("Azimuth", "-999.99", row++, fsize2);

  _volNumLabel = _createStatusLabel("Volume", "0", row++, fsize);
  _sweepNumLabel = _createStatusLabel("Sweep", "0", row++, fsize);
  
  _nSamplesLabel = _createStatusLabel("N samples", "0", row++, fsize);
  _nGatesLabel = _createStatusLabel("N gates", "0", row++, fsize);
  _gateSpacingLabel = _createStatusLabel("Gate space", "0", row++, fsize);

  _pulseWidthLabel = _createStatusLabel("Pulse width", "-9999", row++, fsize);
  _prfLabel = _createStatusLabel("PRF", "-9999", row++, fsize);
  _nyquistLabel = _createStatusLabel("Nyquist", "-9999", row++, fsize);
  _maxRangeLabel = _createStatusLabel("Max range", "-9999", row++, fsize);
  _powerHLabel = _createStatusLabel("Power H", "-9999", row++, fsize);
  _powerVLabel = _createStatusLabel("Power V", "-9999", row++, fsize);

  _sweepModeLabel = _createStatusLabel("Scan mode", "SUR", row++, fsize);
  _polModeLabel = _createStatusLabel("Pol mode", "Single", row++, fsize);
  _prfModeLabel = _createStatusLabel("PRF mode", "Fixed", row++, fsize);

  _latLabel = _createStatusLabel("Lat", "-99.999", row++, fsize);
  _lonLabel = _createStatusLabel("Lon", "-999.999", row++, fsize);
  _altLabel = _createStatusLabel("Alt(km)", "-999.999", row++, fsize);

  QLabel *spacerRow = new QLabel("", _statusPanel);
  _statusLayout->addWidget(spacerRow, row, 0);
  _statusLayout->setRowStretch(row, 1);
  row++;

}

//////////////////////////////////////////////
// create the field panel

void TsAscope::_createFieldPanel()
{
  
  Qt::Alignment alignCenter(Qt::AlignCenter);
  Qt::Alignment alignRight(Qt::AlignRight);
  
  int fsize = _params.label_font_size;
  int fsize2 = _params.label_font_size + 2;
  int fsize4 = _params.label_font_size + 4;
  int fsize6 = _params.label_font_size + 6;

  _fieldPanel = new QGroupBox(_main);
  _fieldGroup = new QButtonGroup;
  _fieldsLayout = new QGridLayout(_fieldPanel);
  _fieldsLayout->setVerticalSpacing(5);

  int row = 0;
  
  _selectedName = _params._fields[0].raw_name;
  _selectedLabel = new QLabel(_selectedName.c_str(), _fieldPanel);
  QFont font6 = _selectedLabel->font();
  font6.setPixelSize(fsize6);
  _selectedLabel->setFont(font6);
  _fieldsLayout->addWidget(_selectedLabel, row, 0, 1, 4, alignCenter);
  row++;

  QFont font4 = _selectedLabel->font();
  font4.setPixelSize(fsize4);
  QFont font2 = _selectedLabel->font();
  font2.setPixelSize(fsize2);
  QFont font = _selectedLabel->font();
  font.setPixelSize(fsize);

  _valueLabel = new QLabel("", _fieldPanel);
  _valueLabel->setFont(font);
  _fieldsLayout->addWidget(_valueLabel, row, 0, 1, 4, alignCenter);
  row++;

  QLabel *fieldHeader = new QLabel("FIELD LIST", _fieldPanel);
  fieldHeader->setFont(font);
  _fieldsLayout->addWidget(fieldHeader, row, 0, 1, 4, alignCenter);
  row++;

  QLabel *nameHeader = new QLabel("Name", _fieldPanel);
  nameHeader->setFont(font);
  _fieldsLayout->addWidget(nameHeader, row, 0, alignCenter);
  QLabel *keyHeader = new QLabel("Key", _fieldPanel);
  keyHeader->setFont(font);
  _fieldsLayout->addWidget(keyHeader, row, 1, alignCenter);
  QLabel *rawHeader = new QLabel("Raw", _fieldPanel);
  rawHeader->setFont(font);
  _fieldsLayout->addWidget(rawHeader, row, 2, alignCenter);
  QLabel *filtHeader = new QLabel("Filt", _fieldPanel);
  filtHeader->setFont(font);
  _fieldsLayout->addWidget(filtHeader, row, 3, alignCenter);
  row++;

  for (int ifield = 0; ifield < _params.fields_n; ifield++) {
    QLabel *label = new QLabel(_fieldPanel);
    label->setFont(font);
    label->setText(_params._fields[ifield].label);
    QLabel *key = new QLabel(_fieldPanel);
    key->setFont(font);
    if (strlen(_params._fields[ifield].shortcut) > 0) {
      char text[4];
      text[0] = _params._fields[ifield].shortcut[0];
      text[1] = '\0';
      key->setText(text);
      char text2[128];
      sprintf(text2, "Hit %s for %s, ALT-%s for %s",
	      text, _params._fields[ifield].raw_name,
	      text, _params._fields[ifield].filtered_name);
      label->setToolTip(text2);
      key->setToolTip(text2);
    }
    QRadioButton *rawButton = new QRadioButton(_fieldPanel);
    if (strlen(_params._fields[ifield].raw_name) == 0) {
      rawButton->setEnabled(false);
    }
    rawButton->setToolTip(_params._fields[ifield].raw_name);
    if (ifield == 0) {
      rawButton->click();
    }
    QRadioButton *filtButton = new QRadioButton(_fieldPanel);
    if (strlen(_params._fields[ifield].filtered_name) == 0) {
      filtButton->setEnabled(false);
    }
    filtButton->setToolTip(_params._fields[ifield].filtered_name);
    _fieldsLayout->addWidget(label, row, 0, alignCenter);
    _fieldsLayout->addWidget(key, row, 1, alignCenter);
    _fieldsLayout->addWidget(rawButton, row, 2, alignCenter);
    _fieldsLayout->addWidget(filtButton, row, 3, alignCenter);
    _fieldGroup->addButton(rawButton, ifield * 2);
    _fieldGroup->addButton(filtButton, ifield * 2 + 1);
    _fieldButtons.push_back(rawButton);
    _fieldButtons.push_back(filtButton);
    row++;
  }

  QLabel *spacerRow = new QLabel("", _fieldPanel);
  _fieldsLayout->addWidget(spacerRow, row, 0);
  _fieldsLayout->setRowStretch(row, 1);
  row++;

  // connect slot for field change

  connect(_fieldGroup, SIGNAL(buttonClicked(int)),
          this, SLOT(_changeField(int)));

  // connect slots for location change

  connect(_ppi, SIGNAL(locationClicked(double, double)),
          this, SLOT(_ppiLocationClicked(double, double)));

  connect(_rhi, SIGNAL(locationClicked(double, double)),
          this, SLOT(_rhiLocationClicked(double, double)));

}
 
///////////////////////////////////////////////////////
// create the field status dialog
//
// This shows the field values at the latest click point

void TsAscope::_createFieldStatusDialog()
{
  
  _fieldStatusDialog = new QDialog(this);
  _fieldStatusDialog->setMinimumSize(100, 100);
  _fieldStatusDialog->setWindowTitle("Field values");

  Qt::Alignment alignCenter(Qt::AlignCenter);
  Qt::Alignment alignRight(Qt::AlignRight);
  
  _fieldDialogLayout = new QGridLayout(_fieldStatusDialog);
  _fieldDialogLayout->setVerticalSpacing(5);

  int row = 0;
  QLabel *mainHeader = new QLabel("CLICK POINT DATA", _fieldStatusDialog);
  _fieldDialogLayout->addWidget(mainHeader, row, 0, 1, 3, alignCenter);
  row++;

  _dateClicked = _createDialogLabel("Date", "9999/99/99", row++);
  _timeClicked = _createDialogLabel("Time", "99:99:99.999", row++);
  _elevClicked = _createDialogLabel("Elevation", "-9999", row++);
  _azClicked = _createDialogLabel("Azimuth", "-9999", row++);
  _gateNumClicked = _createDialogLabel("Gate num", "-9999", row++);
  _rangeClicked = _createDialogLabel("Range", "-9999", row++);

  QLabel *valHeader = new QLabel("FIELD VALUES", _fieldStatusDialog);
  _fieldDialogLayout->addWidget(valHeader, row, 0, 1, 3, alignCenter);
  row++;

  QLabel *nameHeader = new QLabel("Name", _fieldStatusDialog);
  _fieldDialogLayout->addWidget(nameHeader, row, 0, alignCenter);
  QLabel *rawHeader = new QLabel("Raw", _fieldStatusDialog);
  _fieldDialogLayout->addWidget(rawHeader, row, 1, alignCenter);
  QLabel *filtHeader = new QLabel("Filt", _fieldStatusDialog);
  _fieldDialogLayout->addWidget(filtHeader, row, 2, alignCenter);
  row++;

  for (int ifield = 0; ifield < _params.fields_n; ifield++) {

    QLabel *label = new QLabel(_fieldStatusDialog);
    label->setText(_params._fields[ifield].label);

    QLabel *rawVal = new QLabel(_fieldStatusDialog);
    rawVal->setText("-------------");
    _fields[ifield*2].dialogEntry = rawVal;

    QLabel *filtVal = new QLabel(_fieldStatusDialog);
    filtVal->setText("-------------");
    _fields[ifield*2+1].dialogEntry = filtVal;

    _fieldDialogLayout->addWidget(label, row, 0, alignCenter);
    _fieldDialogLayout->addWidget(rawVal, row, 1, alignCenter);
    _fieldDialogLayout->addWidget(filtVal, row, 2, alignCenter);
    row++;
    
  }
  
}

//////////////////////////////////////////////
// create the RHI window

void TsAscope::_createRhiWindow()
{
  // Create the RHI widget with a null parent.  The parent will be reset
  // when the RHI window is created.

  _rhi = new RhiWidget(0, _params);

  _rhi->configure(_nFields, _nGates, _params.background_render_mins, _maxRange);
  _rhi->setRings(true);
  _rhi->setGrids(false);
  _rhi->setAzLines(true);

  // Create the RHI window

  _rhiWindow = new RhiWindow(this, _rhi, _params);
  _rhiWindow->setRadarName(_params.radar_name);
}

//////////////////////////////////////////////
// make a new label with right justification

QLabel *TsAscope::_newLabelRight(const string &text)
{
  QLabel *label = new QLabel;
  label->setText("-----");
  label->setAlignment(Qt::AlignRight);
  return label;
}

//////////////////////////////////////////////////
// create a row in the status panel

QLabel *TsAscope::_createStatusLabel(const string &leftLabel,
				    const string &rightLabel,
				    int row, 
                                    int fontSize)
  
{

  QLabel *left = new QLabel(_statusPanel);
  left->setText(leftLabel.c_str());
  Qt::Alignment alignRight(Qt::AlignRight);
  _statusLayout->addWidget(left, row, 0, alignRight);

  QLabel *right = new QLabel(_statusPanel);
  right->setText(rightLabel.c_str());
  Qt::Alignment alignCenter(Qt::AlignCenter);
  _statusLayout->addWidget(right, row, 1, alignRight);

  if (fontSize > 0) {
    QFont font = left->font();
    font.setPixelSize(fontSize);
    left->setFont(font);
    right->setFont(font);
  }

  return right;
}

//////////////////////////////////////////////////
// create a row in the field status dialog

QLabel *TsAscope::_createDialogLabel(const string &leftLabel,
				    const string &rightLabel,
				    int row, 
                                    int fontSize)
  
{

  QLabel *left = new QLabel(_fieldStatusDialog);
  left->setText(leftLabel.c_str());
  Qt::Alignment alignRight(Qt::AlignRight);
  _fieldDialogLayout->addWidget(left, row, 0, alignRight);

  QLabel *right = new QLabel(_fieldStatusDialog);
  right->setText(rightLabel.c_str());
  Qt::Alignment alignCenter(Qt::AlignCenter);
  _fieldDialogLayout->addWidget(right, row, 1, 1, 2, alignCenter);

  if (fontSize > 0) {
    QFont font = left->font();
    font.setPixelSize(fontSize);
    left->setFont(font);
    right->setFont(font);
  }

  return right;
}

//////////////////////////////////////////////////
// set up reader thread
// returns 0 on success, -1 on failure
  
int TsAscope::_setupReader()
{

  switch (_params.input_mode) {

    case Params::DSR_FMQ_INPUT: {
      DsrFmqReader *dsrReader = new DsrFmqReader(_params);
      _reader = dsrReader;
      break;
    }

    case Params::IWRF_TCP_INPUT: {
      cerr << "ERROR - IWRF TCP input not yet supported" << endl;
      return -1;
      break;
    }
      
  case Params::SIMULATED_RHI_INPUT: {

      SimRhiReader *simReader = new SimRhiReader(_params);
      _reader = simReader;

      vector<SimRhiReader::Field> simFields;
      for (size_t ii = 0; ii < _fields.size(); ii++) {
        SimRhiReader::Field simField;
        simField.name = _fields[ii].name;
        simField.units = _fields[ii].units;
        simField.minVal = _fields[ii].colorMap->rangeMin();
        simField.maxVal = _fields[ii].colorMap->rangeMax();
        simFields.push_back(simField);
      }
      simReader->setFields(simFields);

      break;
    }

    case Params::SIMULATED_INPUT:
    default: {

      SimReader *simReader = new SimReader(_params);
      _reader = simReader;

      vector<SimReader::Field> simFields;
      for (size_t ii = 0; ii < _fields.size(); ii++) {
        SimReader::Field simField;
        simField.name = _fields[ii].name;
        simField.units = _fields[ii].units;
        simField.minVal = _fields[ii].colorMap->rangeMin();
        simField.maxVal = _fields[ii].colorMap->rangeMax();
        simFields.push_back(simField);
      }
      simReader->setFields(simFields);

    }

  } // switch

  return 0;

}

//////////////////////////////////////////////////////////////
// respond to timer events
  
void TsAscope::timerEvent(QTimerEvent *event)
{
//  static size_t beams_processed = 0;
  
  // Handle widget stuff that can't be done at initial setup.  For some reason
  // the widget sizes are off until we get to this point.  There's probably
  // a better way to do this, but I couldn't figure anything out.

  if (_firstTimerEvent) {
    _ppi->resize(_ppiParent->width(), _ppiParent->height());
    
    // Set the minimum size of the second column to the size of the largest
    // label.  This should keep the column from wiggling as the values change.
    // The default values for the labels must be their maximum size for this
    // to work.  This is ugly, but it works.

    int max_width = 0;
    if (max_width < _fixedAngLabel->width()) max_width = _fixedAngLabel->width();
    if (max_width < _elevLabel->width()) max_width = _elevLabel->width();
    if (max_width < _azLabel->width()) max_width = _azLabel->width();
    if (max_width < _volNumLabel->width()) max_width = _volNumLabel->width();
    if (max_width < _sweepNumLabel->width()) max_width = _sweepNumLabel->width();
    if (max_width < _nSamplesLabel->width()) max_width = _nSamplesLabel->width();
    if (max_width < _nGatesLabel->width()) max_width = _nGatesLabel->width();
    if (max_width < _gateSpacingLabel->width()) max_width = _gateSpacingLabel->width();
    if (max_width < _pulseWidthLabel->width()) max_width = _pulseWidthLabel->width();
    if (max_width < _prfLabel->width()) max_width = _prfLabel->width();
    if (max_width < _nyquistLabel->width()) max_width = _nyquistLabel->width();
    if (max_width < _maxRangeLabel->width()) max_width = _maxRangeLabel->width();
    if (max_width < _powerHLabel->width()) max_width = _powerHLabel->width();
    if (max_width < _powerVLabel->width()) max_width = _powerVLabel->width();
    if (max_width < _sweepModeLabel->width()) max_width = _sweepModeLabel->width();
    if (max_width < _polModeLabel->width()) max_width = _polModeLabel->width();
    if (max_width < _prfModeLabel->width()) max_width = _prfModeLabel->width();
    if (max_width < _latLabel->width()) max_width = _latLabel->width();
    if (max_width < _lonLabel->width()) max_width = _lonLabel->width();
    if (max_width < _altLabel->width()) max_width = _altLabel->width();
    
    _statusLayout->setColumnMinimumWidth(1, max_width);
  
    _firstTimerEvent = false;
  }

  if (event->timerId() == _beamTimerId && !_frozen) {

    // get all available beams

    while (true) {

      // get the next ray from the reader queue
      // responsibility for this ray memory passes to
      // this (the master) thread

      RadxVol vol;
      RadxRay *ray = _reader->getNextRay(vol);
      if (ray == NULL) {
        break; // no pending rays
      }

      if (_params.debug >= Params::DEBUG_EXTRA) {
        cerr << "  Got a ray, time, el, az: "
             << DateTime::strm(ray->getTimeSecs()) << ", "
             << ray->getElevationDeg() << ", "
             << ray->getAzimuthDeg() << endl;
        // ray->printWithFieldMeta(cerr);
      }

      // draw the beam
      
      _drawBeam(vol, ray);

//      beams_processed++;
//      if (beams_processed > 1000)
//	exit(0);
      
      // delete the ray

      // delete ray;

    } // while

  }
    
}

//////////////////////////////////////////////////////////////
// raw a beam

void TsAscope::_drawBeam(RadxVol &vol, RadxRay *ray)
  
{
  // do we need to reconfigure the PPI?

  int nGates = ray->getNGates();
  double maxRange = ray->getStartRangeKm() + nGates * ray->getGateSpacingKm();
  if (_nGates != nGates || fabs(_maxRange - maxRange) > 0.001) {
    _nGates = nGates;
    _maxRange = maxRange;
    _ppi->configure(_nFields, _nGates,
		    _params.background_render_mins, _maxRange);
    _rhi->configure(_nFields, _nGates,
		    _params.background_render_mins, _maxRange);
  }

  // create 2D field data vector

  vector< vector<double> > fieldData;
  for (int ifield = 0; ifield < _nFields; ifield++) {
    vector<double> field;
    fieldData.push_back(field);
  }

  // fill data vector

  for (int ifield = 0; ifield < _nFields; ifield++) {
    vector<double> &data = fieldData[ifield];
    RadxField *rfld = ray->getField(_fields[ifield].name);
    if (rfld == NULL) {
      // fill with missing
      for (int igate = 0; igate < _nGates; igate++) {
        data.push_back(-9999);
      }
    } else {
      rfld->convertToFl32();
      const Radx::fl32 *fdata = rfld->getDataFl32();
      const Radx::fl32 missingVal = rfld->getMissingFl32();
      for (int igate = 0; igate < _nGates; igate++, fdata++) {
        Radx::fl32 val = *fdata;
        if (fabs(val - missingVal) < 0.0001) {
          data.push_back(-9999);
        } else {
          data.push_back(*fdata);
        }
      }
    }
  }

  // Store the ray location (which also sets _startAz and _endAz), then
  // draw beam on the PPI or RHI, as appropriate

  if (ray->getSweepMode() == Radx::SWEEP_MODE_RHI ||
      ray->getSweepMode() == Radx::SWEEP_MODE_ELEVATION_SURVEILLANCE) {
    // Store the ray location using the elevation angle and the RHI location
    // table

    double el = 90.0 - ray->getElevationDeg();
    if (el < 0.0)
      el += 360.0;
    _storeRayLoc(ray, el, vol.getRadarBeamWidthDegV(), _rhiRayLoc);

    // Save the angle information for the next iteration

    _prevEl = el;
    _prevAz = -9999.0;
    
    // If this is the first RHI beam we've encountered, automatically open
    // the RHI window.  After this, opening and closing the window will be
    // left to the user.

    if (!_rhiWindowDisplayed) {
      _rhiWindow->show();
      _rhiWindow->resize();
      _rhiWindowDisplayed = true;
    }

    // Add the beam to the display

    _rhi->addBeam(_startAz, _endAz, _nGates, fieldData, _colorMaps);
    _rhiWindow->setAzimuth(ray->getAzimuthDeg());
    _rhiWindow->setElevation(ray->getElevationDeg());
    
  } else {
    // Store the ray location using the azimuth angle and the PPI location
    // table

    double az = ray->getAzimuthDeg();
    _storeRayLoc(ray, az, vol.getRadarBeamWidthDegH(), _ppiRayLoc);

    // Save the angle information for the next iteration

    _prevAz = az;
    _prevEl = -9999.0;

    // Add the beam to the display

    _ppi->addBeam(_startAz, _endAz, _nGates, fieldData, _colorMaps);
  }
  
  // set time etc

  char text[1024];


  QString prev_radar_name = _radarName->text();
  
  string rname(vol.getInstrumentName());
  if (_params.override_radar_name) rname = _params.radar_name;
  if (_params.display_site_name) {
    rname += ":";
    if (_params.override_site_name) {
      rname += _params.site_name;
    } else {
      rname += vol.getSiteName();
    }
  }
  _radarName->setText(rname.c_str());

  if (prev_radar_name != _radarName->text())
  {
    setRadarName(rname);
    _rhiWindow->setRadarName(rname);
  }
  
  DateTime rayTime(ray->getTimeSecs());
  sprintf(text, "%.4d/%.2d/%.2d",
          rayTime.getYear(), rayTime.getMonth(), rayTime.getDay());
  _dateLabel->setText(text);

  sprintf(text, "%.2d:%.2d:%.2d.%.3d",
          rayTime.getHour(), rayTime.getMin(), rayTime.getSec(),
          ((int) ray->getNanoSecs() / 1000000));
  _timeLabel->setText(text);
  
//   sprintf(text, "%.6d", (int) (ray->getNanoSecs() / 1000));
//   _microSecsLabel->setText(text);
  
  sprintf(text, "%d", ray->getVolumeNumber());
  _volNumLabel->setText(text);
  
  sprintf(text, "%d", ray->getSweepNumber());
  _sweepNumLabel->setText(text);
  
  sprintf(text, "%6.2f", ray->getFixedAngleDeg());
  _fixedAngLabel->setText(text);
  
  sprintf(text, "%6.2f", ray->getElevationDeg());
  _elevLabel->setText(text);
  
  sprintf(text, "%6.2f", ray->getAzimuthDeg());
  _azLabel->setText(text);
  
  sprintf(text, "%d", (int) ray->getNSamples());
  _nSamplesLabel->setText(text);
  
  sprintf(text, "%d", (int) ray->getNGates());
  _nGatesLabel->setText(text);
  
  sprintf(text, "%.3f", ray->getGateSpacingKm());
  _gateSpacingLabel->setText(text);
  
  sprintf(text, "%.2f", ray->getPulseWidthUsec());
  _pulseWidthLabel->setText(text);

  if (ray->getPrtSec() <= 0) {
    sprintf(text, "-99");
  } else {
    sprintf(text, "%d", (int) ((1.0 / ray->getPrtSec()) * 10.0 + 0.5) / 10);
  }
  _prfLabel->setText(text);
  
  sprintf(text, "%.1f", ray->getNyquistMps());
  _nyquistLabel->setText(text);
  
  sprintf(text, "%.1f", ray->getUnambigRangeKm());
  _maxRangeLabel->setText(text);
  
  sprintf(text, "%.1f", ray->getMeasXmitPowerDbmH());
  _powerHLabel->setText(text);
  
  sprintf(text, "%.1f", ray->getMeasXmitPowerDbmV());
  _powerVLabel->setText(text);

  switch (ray->getSweepMode()) {
    case Radx::SWEEP_MODE_SECTOR: {
      _sweepModeLabel->setText("sector"); break;
    }
    case Radx::SWEEP_MODE_COPLANE: {
      _sweepModeLabel->setText("coplane"); break;
    }
    case Radx::SWEEP_MODE_RHI: {
      _sweepModeLabel->setText("RHI"); break;
    }
    case Radx::SWEEP_MODE_VERTICAL_POINTING: {
      _sweepModeLabel->setText("vert"); break;
    }
    case Radx::SWEEP_MODE_IDLE: {
      _sweepModeLabel->setText("idle"); break;
    }
    case Radx::SWEEP_MODE_AZIMUTH_SURVEILLANCE:
    case Radx::SWEEP_MODE_ELEVATION_SURVEILLANCE: {
      _sweepModeLabel->setText("SUR"); break;
    }
    case Radx::SWEEP_MODE_SUNSCAN: {
      _sweepModeLabel->setText("sunscan"); break;
    }
    case Radx::SWEEP_MODE_POINTING: {
      _sweepModeLabel->setText("point"); break;
    }
    case Radx::SWEEP_MODE_CALIBRATION: {
      _sweepModeLabel->setText("cal"); break;
    }
    default: {
      _sweepModeLabel->setText("unknown");
    }
  }

  _polModeLabel->setText(Radx::polarizationModeToStr
                         (ray->getPolarizationMode()).c_str());

  _prfModeLabel->setText(Radx::prtModeToStr
                         (ray->getPrtMode()).c_str());
  
  sprintf(text, "%.3f", vol.getLatitudeDeg());
  _latLabel->setText(text);
  
  sprintf(text, "%.3f", vol.getLongitudeDeg());
  _lonLabel->setText(text);
  
  sprintf(text, "%.4f", vol.getAltitudeKm());
  _altLabel->setText(text);
  
}

void TsAscope::resizeEvent(QResizeEvent *event)
{
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "resizeEvent: " << event << endl;
  }
  emit frameResized(_ppiParent->width(), _ppiParent->height());
}

// void TsAscope::_contextMenuEvent(QContextMenuEvent *event)
// {
//   QMenu menu(this);
//   // menu.addAction(_printAct);
//   menu.addAction(_ringsOnAct);
//   menu.addAction(_ringsOffAct);
//   menu.addAction(_gridsOnAct);
//   menu.addAction(_gridsOffAct);
//   menu.addAction(_azLinesOnAct);
//   menu.addAction(_azLinesOffAct);
//   menu.exec(event->globalPos());
// }

void TsAscope::_freeze()
{
  if (_frozen) {
    _frozen = false;
    _freezeAct->setText("Freeze");
    _freezeAct->setStatusTip(tr("Click to freeze display, or hit ESC"));
  } else {
    _frozen = true;
    _freezeAct->setText("Unfreeze");
    _freezeAct->setStatusTip(tr("Click to unfreeze display, or hit ESC"));
  }
}

void TsAscope::_showFields()
{
  if (_fieldStatusDialog) {
    if (_fieldStatusDialog->isVisible()) {
      _fieldStatusDialog->setVisible(false);
    } else {
      QPoint pos;
      pos.setX(x() + width() + 10);
      pos.setY(y());
      _fieldStatusDialog->move(pos);
      _fieldStatusDialog->setVisible(true);
      _fieldStatusDialog->raise();
    }
  }
}

void TsAscope::_unzoom()
{
  _ppi->resetView();
}

// void TsAscope::_print()
// {
//   _infoLabel->setText(tr("Invoked <b>File|Print</b>"));
// }

void TsAscope::_howto()
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

void TsAscope::_about()
{
  // _infoLabel->setText(tr("Invoked <b>Help|About</b>"));
  QMessageBox::about(this, tr("About Menu"),
		     tr("TsAscope is an engineering display for beam-by-beam radar data."));
}

void TsAscope::_aboutQt()
{
  // _infoLabel->setText(tr("Invoked <b>Help|About Qt</b>"));
}

void TsAscope::_createActions()
{

  _freezeAct = new QAction(tr("Freeze"), this);
  _freezeAct->setShortcut(tr("Esc"));
  _freezeAct->setStatusTip(tr("Freeze display"));
  connect(_freezeAct, SIGNAL(triggered()), this, SLOT(_freeze()));
  
  _showFieldsAct = new QAction(tr("Show Click"), this);
  _showFieldsAct->setStatusTip(tr("Show click value dialog"));
  connect(_showFieldsAct, SIGNAL(triggered()), this, SLOT(_showFields()));

  _unzoomAct = new QAction(tr("Unzoom"), this);
  _unzoomAct->setStatusTip(tr("Unzoom to original view"));
  connect(_unzoomAct, SIGNAL(triggered()), this, SLOT(_unzoom()));

  _clearAct = new QAction(tr("Clear"), this);
  _clearAct->setStatusTip(tr("Clear data"));
  connect(_clearAct, SIGNAL(triggered()), _ppi, SLOT(clear()));
  connect(_clearAct, SIGNAL(triggered()), _rhi, SLOT(clear()));

//   _printAct = new QAction(tr("&Print..."), this);
//   _printAct->setShortcuts(QKeySequence::Print);
//   _printAct->setStatusTip(tr("Print the document"));
//   connect(_printAct, SIGNAL(triggered()), this, SLOT(_print()));

  _exitAct = new QAction(tr("E&xit"), this);
  _exitAct->setShortcut(tr("Ctrl+Q"));
  _exitAct->setStatusTip(tr("Exit the application"));
  connect(_exitAct, SIGNAL(triggered()), this, SLOT(close()));

  _ringsAct = new QAction(tr("Range Rings"), this);
  _ringsAct->setStatusTip(tr("Turn range rings on/off"));
  _ringsAct->setCheckable(true);
  _ringsAct->setChecked(true);
  connect(_ringsAct, SIGNAL(triggered(bool)),
	  _ppi, SLOT(setRings(bool)));

  _gridsAct = new QAction(tr("Grids"), this);
  _gridsAct->setStatusTip(tr("Turn range grids on/off"));
  _gridsAct->setCheckable(true);
  _gridsAct->setChecked(false);
  connect(_gridsAct, SIGNAL(triggered(bool)),
	  _ppi, SLOT(setGrids(bool)));

  _azLinesAct = new QAction(tr("Az Lines"), this);
  _azLinesAct->setStatusTip(tr("Turn range azLines on/off"));
  _azLinesAct->setCheckable(true);
  _azLinesAct->setChecked(true);
  connect(_azLinesAct, SIGNAL(triggered(bool)),
	  _ppi, SLOT(setAzLines(bool)));

  _showRhiAct = new QAction(tr("Show RHI Window"), this);
  _showRhiAct->setStatusTip(tr("Show the RHI Window"));
  connect(_showRhiAct, SIGNAL(triggered()), _rhiWindow, SLOT(show()));
  
   _howtoAct = new QAction(tr("&Howto"), this);
   _howtoAct->setStatusTip(tr("Show the application's Howto box"));
   connect(_howtoAct, SIGNAL(triggered()), this, SLOT(_howto()));

   _aboutAct = new QAction(tr("&About"), this);
   _aboutAct->setStatusTip(tr("Show the application's About box"));
   connect(_aboutAct, SIGNAL(triggered()), this, SLOT(_about()));

   _aboutQtAct = new QAction(tr("About &Qt"), this);
   _aboutQtAct->setStatusTip(tr("Show the Qt library's About box"));
   connect(_aboutQtAct, SIGNAL(triggered()), qApp, SLOT(aboutQt()));
   connect(_aboutQtAct, SIGNAL(triggered()), this, SLOT(_aboutQt()));

 }

 void TsAscope::_createMenus()
 {

   _fileMenu = menuBar()->addMenu(tr("&File"));
   // _fileMenu->addAction(_printAct);
   _fileMenu->addSeparator();
   _fileMenu->addAction(_exitAct);

   _viewMenu = menuBar()->addMenu(tr("&View"));
   _viewMenu->addAction(_ringsAct);
   _viewMenu->addAction(_gridsAct);
   _viewMenu->addAction(_azLinesAct);
   _viewMenu->addSeparator();
   _viewMenu->addAction(_showRhiAct);

   menuBar()->addAction(_freezeAct);
   menuBar()->addAction(_showFieldsAct);
   menuBar()->addAction(_unzoomAct);
   menuBar()->addAction(_clearAct);

   _helpMenu = menuBar()->addMenu(tr("&Help"));
   _helpMenu->addAction(_howtoAct);
   _helpMenu->addAction(_aboutAct);
   _helpMenu->addAction(_aboutQtAct);

}

///////////////////////////////////////////////////////////
// respond to change field request from field button group

void TsAscope::_changeField(int fieldId)

{
  if (_params.debug) {
    cerr << "Changing to field id: " << fieldId << endl;
  }

  // if we click the already-selected field, go back to previous field

  if (_fieldNum == fieldId && _prevFieldNum >= 0) {
    QRadioButton *button = (QRadioButton *) _fieldGroup->button(_prevFieldNum);
    button->click();
    return;
  }

  _prevFieldNum = _fieldNum;
  _fieldNum = fieldId;
  
  _ppi->selectVar(_fieldNum);
  _rhi->selectVar(_fieldNum);
  _colorBar->setColorMap(_colorMaps[_fieldNum]);
  QRadioButton *button = _fieldButtons[_fieldNum];
  _selectedName = button->toolTip().toAscii().data();
  _selectedLabel->setText(_selectedName.c_str());
  
  for (size_t ii = 0; ii < _fields.size(); ii++) {
    if (_fields[ii].name == _selectedName) {
      char text[128];
      if (_fields[ii].selectValue > -9990) {
	sprintf(text, "%g %s", _fields[ii].selectValue,
		_fields[ii].units.c_str());
      } else {
	text[0] = '\0';
      }
      _valueLabel->setText(text);
    }
  }

}

///////////////////////////////////////////////////
// respond to a change in click location on the PPI

void TsAscope::_ppiLocationClicked(double xkm, double ykm)

{
  _locationClicked(xkm, ykm, _ppiRayLoc);
}

///////////////////////////////////////////////////
// respond to a change in click location on the RHI

void TsAscope::_rhiLocationClicked(double xkm, double ykm)

{
  _locationClicked(xkm, ykm, _rhiRayLoc);
}

///////////////////////////////////////////////////
// respond to a change in click location on one of the windows

void TsAscope::_locationClicked(double xkm, double ykm,
			       RayLoc *ray_loc)

{

  cerr << "*** Entering TsAscope::_locationClicked()" << endl;
  
  // find the relevant ray

  double azDeg = 0.0;
  if (xkm != 0 || ykm != 0) {
    azDeg = atan2(xkm, ykm) * RAD_TO_DEG;
    if (azDeg < 0) {
      azDeg += 360.0;
    }
  }
  cerr << "    azDeg = " << azDeg << endl;
  
  int rayIndex = (int) (azDeg * RAY_LOC_RES);
  cerr << "    rayIndex = " << rayIndex << endl;
  
  const RadxRay *ray = ray_loc[rayIndex].ray;
  if (ray == NULL) {
    cerr << "    No ray data yet..." << endl;
    cerr << "      active = " << ray_loc[rayIndex].active << endl;
    cerr << "      master = " << ray_loc[rayIndex].master << endl;
    cerr << "      startIndex = " << ray_loc[rayIndex].startIndex << endl;
    cerr << "      endIndex = " << ray_loc[rayIndex].endIndex << endl;
    
    // no ray data yet
    return;
  }

  double range = sqrt(xkm * xkm + ykm * ykm);
  int gate = (int) 
    ((range - ray->getStartRangeKm()) / ray->getGateSpacingKm() + 0.5);

  if (_params.debug) {
    cerr << "Clicked on location: xkm, ykm: " << xkm << ", " << ykm << endl;
    cerr << "  az, index: " << azDeg << ", " << rayIndex << endl;
    cerr << "  range, gate: " << range << ", " << gate << endl;
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      ray->print(cerr);
    }
  }

  DateTime rayTime(ray->getTimeSecs());
  char text[256];
  sprintf(text, "%.4d/%.2d/%.2d",
          rayTime.getYear(), rayTime.getMonth(), rayTime.getDay());
  _dateClicked->setText(text);

  sprintf(text, "%.2d:%.2d:%.2d.%.3d",
          rayTime.getHour(), rayTime.getMin(), rayTime.getSec(),
          ((int) (ray->getNanoSecs() / 1000000)));
  _timeClicked->setText(text);
  
  sprintf(text, "%6.2f", ray->getElevationDeg());
  _elevClicked->setText(text);
  
  sprintf(text, "%6.2f", ray->getAzimuthDeg());
  _azClicked->setText(text);
  
  sprintf(text, "%d", gate);
  _gateNumClicked->setText(text);
  
  sprintf(text, "%6.2f", range);
  _rangeClicked->setText(text);
  
  for (size_t ii = 0; ii < _fields.size(); ii++) {
    _fields[ii].selectValue = -9999;
  }
  
  for (size_t ifield = 0; ifield < ray->getNFields(); ifield++) {
    const RadxField *field = ray->getField(ifield);
    Radx::fl32 *data = (Radx::fl32 *) field->getData();
    double val = data[gate];
    const string fieldName = field->getName();
    const string fieldUnits = field->getUnits();
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "Field name, selected name: "
	   << fieldName << ", "
	   << _selectedName << endl;
    }
    if (fieldName == _selectedName) {
      char text[128];
      sprintf(text, "%g %s", val, fieldUnits.c_str());
      _valueLabel->setText(text);
    }
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "Field name, units, val: "
	   << field->getName() << ", "
	   << field->getUnits() << ", "
	   << val << endl;
    }
    for (size_t ii = 0; ii < _fields.size(); ii++) {
      if (_fields[ii].name == fieldName) {
	_fields[ii].selectValue = val;
        char text[128];
        if (fabs(val) > 10) {
          sprintf(text, "%.2f", val);
        } else {
          sprintf(text, "%.3f", val);
        }
        _fields[ii].dialogEntry->setText(text);
      }
    }

  } // ifield

}

///////////////////////////////////////////////////////////
// store ray location

void TsAscope::_storeRayLoc(const RadxRay *ray, const double az,
			   const double beam_width, RayLoc *ray_loc)
{
  // Determine the extent of this ray

  if (ray->getIsIndexed())
  {
    double half_angle = ray->getAngleResDeg() / 2.0;
    _startAz = az - half_angle;
    _endAz = az + half_angle;
  }
  else
  {
    double max_half_angle = beam_width / 2.0;
    double prev_offset = max_half_angle;
    if (_prevAz >= 0.0)
    {
      double az_diff = az - _prevAz;
      if (az_diff < 0.0)
	az_diff += 360.0;
      double half_az_diff = az_diff / 2.0;
	
      if (prev_offset > half_az_diff)
	prev_offset = half_az_diff;
    }
      
    _startAz = az - prev_offset;
    _endAz = az + max_half_angle;
  }
    
  // store
    
  int startIndex = (int) (_startAz * RAY_LOC_RES);
  int endIndex = (int) (_endAz * RAY_LOC_RES + 1);

  // Clear out any rays in the locations list that are overlapped by the
  // new ray
    
  _clearRayOverlap(startIndex, endIndex, ray_loc);

  // Set the locations associated with this ray

  for (int ii = startIndex; ii <= endIndex; ii++) {
    ray_loc[ii].ray = ray;
    ray_loc[ii].active = true;
    ray_loc[ii].master = false;
    ray_loc[ii].startIndex = startIndex;
    ray_loc[ii].endIndex = endIndex;
  }

  // indicate which ray is the master
  // i.e. it is responsible for ray memory
    
  int midIndex = (int) (az * RAY_LOC_RES);
  ray_loc[midIndex].master = true;

}

/////////////////////////////////////////////////////
// clear any locations that are overlapped by the given ray

void TsAscope::_clearRayOverlap(const int start_index, const int end_index,
			       RayLoc *ray_loc)
{
  // Loop through the ray locations, clearing out old information

  int i = start_index;
  
  while (i <= end_index)
  {
    RayLoc &loc = ray_loc[i];
    
    // If this location isn't active, we can skip it

    if (!loc.active)
    {
      ++i;
      continue;
    }
    
    int loc_start_index = loc.startIndex;
    int loc_end_index = loc.endIndex;
      
    // If we get here, this location is active.  We now have 4 possible
    // situations:

    if (loc.startIndex < start_index && loc.endIndex <= end_index)
    {
      // The overlap area covers the end of the current beam.  Reduce the
      // current beam down to just cover the area before the overlap area.

      for (int j = start_index; j <= loc_end_index; ++j)
      {
	// If the master is in the overlap area, then it needs to be moved
	// outside of this area

	if (ray_loc[j].master)
	  ray_loc[start_index-1].master = true;
	
	ray_loc[j].ray = NULL;
	ray_loc[j].active = false;
	ray_loc[j].master = false;
      }

      // Update the end indices for the remaining locations in the current
      // beam

      for (int j = loc_start_index; j < start_index; ++j)
	ray_loc[j].endIndex = start_index - 1;
    }
    else if (loc.startIndex < start_index && loc.endIndex > end_index)
    {
      // The current beam is bigger than the overlap area.  This should never
      // happen, so go ahead and just clear out the locations for the current
      // beam.

      for (int j = loc_start_index; j <= loc_end_index; ++j)
      {
	if (ray_loc[j].master && ray_loc[j].ray)
	  delete ray_loc[j].ray;
	
	ray_loc[j].ray = NULL;
	ray_loc[j].active = false;
	ray_loc[j].master = false;
      }
    }
    else if (loc.endIndex > end_index)
    {
      // The overlap area covers the beginning of the current beam.  Reduce the
      // current beam down to just cover the area after the overlap area.

      for (int j = loc_start_index; j <= end_index; ++j)
      {
	// If the master is in the overlap area, then it needs to be moved
	// outside of this area

	if (ray_loc[j].master)
	  ray_loc[end_index+1].master = true;
	
	ray_loc[j].ray = NULL;
	ray_loc[j].active = false;
	ray_loc[j].master = false;
      }

      // Update the start indices for the remaining locations in the current
      // beam

      for (int j = end_index + 1; j <= loc_end_index; ++j)
	ray_loc[j].startIndex = end_index + 1;
    }
    else
    {
      // The current beam is completely covered by the overlap area.  Clear
      // out all of the locations for the current beam.

      for (int j = loc_start_index; j <= loc_end_index; ++j)
      {
	if (ray_loc[j].master && ray_loc[j].ray)
	  delete ray_loc[j].ray;
	
	ray_loc[j].ray = NULL;
	ray_loc[j].active = false;
	ray_loc[j].master = false;
      }
    }
    
    i = loc_end_index + 1;
  } /* endwhile - i */
  
}

////////////////////////////////////////////////////////////////
void TsAscope::keyPressEvent(QKeyEvent * e)
{

  // get key pressed

  Qt::KeyboardModifiers mods = e->modifiers();
  char keychar = e->text().toAscii().data()[0];

  if (_params.debug) {
    cerr << "Clicked key: " << keychar << ":" << (int) keychar << endl;
  }

  // for '.', swap with previous field

  if (keychar == '.') {
    QRadioButton *button = (QRadioButton *) _fieldGroup->button(_prevFieldNum);
    button->click();
    return;
  }
  
  // for ESC, freeze / unfreeze

  if (keychar == 27) {
    _freezeAct->trigger();
    return;
  }
  
  // check for short-cut keys to fields

  for (int ifield = 0; ifield < _params.fields_n; ifield++) {
    
    char shortcut = 0;
    if (strlen(_params._fields[ifield].shortcut) > 0) {
      shortcut = _params._fields[ifield].shortcut[0];
    }

    if (shortcut == keychar) {
      int fieldNum = ifield * 2;
      if (mods & Qt::AltModifier) {
	fieldNum++;
      }
      if (_params.debug) {
	cerr << "Pressed key for field label: "
	     << _params._fields[ifield].label << endl;
	cerr << "  fieldNum: " << fieldNum << endl;
	cerr << "  fieldName: " << _fields[fieldNum].name << endl;
      }
      if (_fields[fieldNum].name.size() > 0) {
	QRadioButton *button = (QRadioButton *) _fieldGroup->button(fieldNum);
	button->click();
      }
    }

  }

}

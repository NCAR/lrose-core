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
// HawkEyeGl.cc
//
// HawkEyeGl object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// July 2006
//
///////////////////////////////////////////////////////////////
//
// HawkEyeGl is a simple test app for Qt
//
///////////////////////////////////////////////////////////////

#include "HawkEyeGl.hh"
#include "PPI.hh"
#include "ColorMap.hh"
#include "ColorBar.hh"
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
#include <toolsa/DateTime.hh>

using namespace std;
bool HawkEyeGl::_firstTimerEvent = true;

// Constructor

HawkEyeGl::HawkEyeGl(int argc, char **argv, QWidget *parent) :
        QMainWindow(parent),
        _args("HawkEyeGl"),
        _argc(argc),
        _argv(argv)

{

  OK = 1;
  _beamTimerId = 0;
  _frozen = false;
  _ppi = NULL;
  _reader = NULL;
  _prevEndAz = -9999;
  _rays = NULL;
  _fieldNum = 0;
  _prevFieldNum = -1;

  // set programe name

  _progName = strdup("HawkEyeGl");
  
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

  // set up ray locator

  _rays = new RayLoc[RAY_LOC_N];
  _rayLoc = _rays + RAY_LOC_OFFSET;

  // set initial field to 0

  _changeField(0);

}

// destructor

HawkEyeGl::~HawkEyeGl()

{

  if (_ppi) {
    delete _ppi;
  }

  if (_reader) {
    delete _reader;
  }

  if (_rays) {
    delete[] _rays;
  }
  
}

//////////////////////////////////////////////////
// Run

int HawkEyeGl::Run(QApplication &app)
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
  
int HawkEyeGl::_setupColorMaps()
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
  
void HawkEyeGl::_setupWindows()
{

  // set up windows

  _main = new QFrame(this);
  setCentralWidget(_main);
  
  // ppi - main window

  _ppiParent = new QFrame(_main);
  _ppiParent->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

  // configure the PPI

  _ppi = new PPI(_ppiParent, _params);
  _ppi->configure(_nFields, _nGates, _maxRange * 2.0);
  _ppi->grids(false);
  _ppi->azLines(true);

  // info label at bottom - remove later
  //   _infoLabel = new QLabel(tr("<i>Choose a menu option, or right-click to "
  //                              "invoke a context menu</i>"), _main);
  //   _infoLabel->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
  //   _infoLabel->setAlignment(Qt::AlignCenter);

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

  setWindowTitle(tr("HAWK-EYE"));
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

void HawkEyeGl::_createStatusPanel()
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


  // other labels

  _fixedAngLabel = _createStatusLabel("Fixed ang", "-9999", row++, fsize2);
  _elevLabel = _createStatusLabel("Elevation", "-9999", row++, fsize2);
  _azLabel = _createStatusLabel("Azimuth", "-9999", row++, fsize2);

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

  _latLabel = _createStatusLabel("Lat", "-9999", row++, fsize);
  _lonLabel = _createStatusLabel("Lon", "-9999", row++, fsize);
  _altLabel = _createStatusLabel("Alt(km)", "-9999", row++, fsize);

  QLabel *spacerRow = new QLabel("", _statusPanel);
  _statusLayout->addWidget(spacerRow, row, 0);
  _statusLayout->setRowStretch(row, 1);
  row++;

}

//////////////////////////////////////////////
// create the field panel

void HawkEyeGl::_createFieldPanel()
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

  // connect slot for location change

  connect(_ppi, SIGNAL(locationClicked(double, double)),
          this, SLOT(_locationClicked(double, double)));

  
}
 
///////////////////////////////////////////////////////
// create the field status dialog
//
// This shows the field values at the latest click point

void HawkEyeGl::_createFieldStatusDialog()
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
// make a new label with right justification

QLabel *HawkEyeGl::_newLabelRight(const string &text)
{
  QLabel *label = new QLabel;
  label->setText("-----");
  label->setAlignment(Qt::AlignRight);
  return label;
}

//////////////////////////////////////////////////
// create a row in the status panel

QLabel *HawkEyeGl::_createStatusLabel(const string &leftLabel,
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

QLabel *HawkEyeGl::_createDialogLabel(const string &leftLabel,
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
  
int HawkEyeGl::_setupReader()
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
  
void HawkEyeGl::timerEvent(QTimerEvent *event)
{

  if (_firstTimerEvent) {
    _resizePpi();
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

      // draw the beam
      
      _drawBeam(vol, ray);

      // delete the ray

      // delete ray;

    } // while

  }
    
}

//////////////////////////////////////////////////////////////
// raw a beam

void HawkEyeGl::_drawBeam(RadxVol &vol, RadxRay *ray)
  
{

  // do we need to reconfigure the PPI?

  int nGates = ray->getNGates();
  double maxRange = ray->getStartRangeKm() + nGates * ray->getGateSpacingKm();
  if (_nGates != nGates || fabs(_maxRange - maxRange) > 0.001) {
    _nGates = nGates;
    _maxRange = maxRange;
    _ppi->configure(_nFields, _nGates, _maxRange * 2.0);
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

  // store the ray location - also computes start and end az

  _storeRayLoc(ray);

  // draw beam on PPI

  _ppi->addBeam(_startAz, _endAz, _nGates, fieldData, _colorMaps);

  // set time etc

  char text[1024];


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

void HawkEyeGl::resizeEvent(QResizeEvent *event)
{
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "resizeEvent: " << event << endl;
  }
  _resizePpi();
}

void HawkEyeGl::_resizePpi()
{
  int squareSize = _ppiParent->width();
  if (_ppiParent->height() < squareSize) {
    squareSize = _ppiParent->height();
  }
  _ppi->resize(squareSize, squareSize);
}

// void HawkEyeGl::_contextMenuEvent(QContextMenuEvent *event)
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

void HawkEyeGl::_freeze()
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

void HawkEyeGl::_showFields()
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

void HawkEyeGl::_unzoom()
{
  _ppi->resetView();
}

// void HawkEyeGl::_print()
// {
//   _infoLabel->setText(tr("Invoked <b>File|Print</b>"));
// }

void HawkEyeGl::_ringsOn()
{
  if (_ppi) _ppi->rings(true);
}

void HawkEyeGl::_ringsOff()
{
  if (_ppi) _ppi->rings(false);
}

void HawkEyeGl::_gridsOn()
{
  if (_ppi) _ppi->grids(true);
}

void HawkEyeGl::_gridsOff()
{
  if (_ppi) _ppi->grids(false);
}

void HawkEyeGl::_azLinesOn()
{
  if (_ppi) _ppi->azLines(true);
}

void HawkEyeGl::_azLinesOff()
{
  if (_ppi) _ppi->azLines(false);
}

void HawkEyeGl::_howto()
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

void HawkEyeGl::_about()
{
  // _infoLabel->setText(tr("Invoked <b>Help|About</b>"));
  QMessageBox::about(this, tr("About Menu"),
		     tr("HawkEyeGl is an engineering display for beam-by-beam radar data."));
}

void HawkEyeGl::_aboutQt()
{
  // _infoLabel->setText(tr("Invoked <b>Help|About Qt</b>"));
}

void HawkEyeGl::_createActions()
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

//   _printAct = new QAction(tr("&Print..."), this);
//   _printAct->setShortcuts(QKeySequence::Print);
//   _printAct->setStatusTip(tr("Print the document"));
//   connect(_printAct, SIGNAL(triggered()), this, SLOT(_print()));

  _exitAct = new QAction(tr("E&xit"), this);
  _exitAct->setShortcut(tr("Ctrl+Q"));
  _exitAct->setStatusTip(tr("Exit the application"));
  connect(_exitAct, SIGNAL(triggered()), this, SLOT(close()));

  _ringsOnAct = new QAction(tr("Rings on"), this);
  _ringsOnAct->setStatusTip(tr("Turn range rings on"));
  connect(_ringsOnAct, SIGNAL(triggered()), this, SLOT(_ringsOn()));

  _ringsOffAct = new QAction(tr("Rings off"), this);
  _ringsOffAct->setStatusTip(tr("Turn range rings off"));
  connect(_ringsOffAct, SIGNAL(triggered()), this, SLOT(_ringsOff()));

  _gridsOnAct = new QAction(tr("Grids on"), this);
  _gridsOnAct->setStatusTip(tr("Turn range grids on"));
  connect(_gridsOnAct, SIGNAL(triggered()), this, SLOT(_gridsOn()));

  _gridsOffAct = new QAction(tr("Grids off"), this);
  _gridsOffAct->setStatusTip(tr("Turn range grids off"));
  connect(_gridsOffAct, SIGNAL(triggered()), this, SLOT(_gridsOff()));

  _azLinesOnAct = new QAction(tr("AzLines on"), this);
  _azLinesOnAct->setStatusTip(tr("Turn range azLines on"));
  connect(_azLinesOnAct, SIGNAL(triggered()), this, SLOT(_azLinesOn()));

  _azLinesOffAct = new QAction(tr("AzLines off"), this);
  _azLinesOffAct->setStatusTip(tr("Turn range azLines off"));
  connect(_azLinesOffAct, SIGNAL(triggered()), this, SLOT(_azLinesOff()));

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

void HawkEyeGl::_createMenus()
{

  _fileMenu = menuBar()->addMenu(tr("&File"));
  // _fileMenu->addAction(_printAct);
  _fileMenu->addSeparator();
  _fileMenu->addAction(_exitAct);

  _viewMenu = menuBar()->addMenu(tr("&View"));
  _viewMenu->addAction(_ringsOnAct);
  _viewMenu->addAction(_ringsOffAct);
  _viewMenu->addAction(_gridsOnAct);
  _viewMenu->addAction(_gridsOffAct);
  _viewMenu->addAction(_azLinesOnAct);
  _viewMenu->addAction(_azLinesOffAct);
  _viewMenu->addSeparator();

  menuBar()->addAction(_freezeAct);
  menuBar()->addAction(_showFieldsAct);
  menuBar()->addAction(_unzoomAct);

  _helpMenu = menuBar()->addMenu(tr("&Help"));
  _helpMenu->addAction(_howtoAct);
  _helpMenu->addAction(_aboutAct);
  _helpMenu->addAction(_aboutQtAct);

}

///////////////////////////////////////////////////////////
// respond to change field request from field button group

void HawkEyeGl::_changeField(int fieldId)

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
// respond to a change in click loction on the PPI

void HawkEyeGl::_locationClicked(double xkm, double ykm)

{

  // find the relevant ray

  double azDeg = 0.0;
  if (xkm != 0 || ykm != 0) {
    azDeg = atan2(xkm, ykm) * PPI::radToDeg;
    if (azDeg < 0) {
      azDeg += 360.0;
    }
  }
  int rayIndex = (int) (azDeg * RAY_LOC_RES);
  const RadxRay *ray = _rayLoc[rayIndex].ray;
  if (ray == NULL) {
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

void HawkEyeGl::_storeRayLoc(const RadxRay *ray)
{

  int startIndex = 0;
  int endIndex = 0;
  int midIndex = 0;
    
  if (ray->getSweepMode() == Radx::SWEEP_MODE_RHI ||
      ray->getSweepMode() == Radx::SWEEP_MODE_ELEVATION_SURVEILLANCE) {

    // RHI mode

    double el = 90.0 - ray->getElevationDeg();
    midIndex = (int) (el * RAY_LOC_RES);
    
    // first clear any locations associated with this ray
    
    _clearRayLoc(midIndex);
    
    // compute start and end az
    
    // ******** FIX FIX *********
    // double deltaEl = RadarComplex::diffDeg(az, _prevEndAz);
    //   if (fabs(deltaAz) < 2.0) {
    //     _startAz = _prevEndAz;
    //     _endAz = az + deltaAz;
    //   } else {
    _startAz = el - 0.5;
    _endAz = el + 0.5;
    //   }
    //   cerr << "2222222 az, _startAz, _endAz, prevEndAz: "
    //        << az << ", "
    //        << _startAz << ", "
    //        << _endAz << ", "
    //        << _prevEndAz << endl;
    // _prevEndAz = _endAz;
    
    // store
    
    startIndex = (int) (_startAz * RAY_LOC_RES);
    endIndex = (int) (_endAz * RAY_LOC_RES + 1);

  } else {

    // PPI mode

    double az = ray->getAzimuthDeg();
    midIndex = (int) (az * RAY_LOC_RES);
    
    // first clear any locations associated with this ray
    
    _clearRayLoc(midIndex);
    
    // compute start and end az
    
    // ******** FIX FIX *********
    // double deltaAz = RadarComplex::diffDeg(az, _prevEndAz);
    //   if (fabs(deltaAz) < 2.0) {
    //     _startAz = _prevEndAz;
    //     _endAz = az + deltaAz;
    //   } else {
    _startAz = az - 0.5;
    _endAz = az + 0.5;
    //   }
    //   cerr << "2222222 az, _startAz, _endAz, prevEndAz: "
    //        << az << ", "
    //        << _startAz << ", "
    //        << _endAz << ", "
    //        << _prevEndAz << endl;
    // _prevEndAz = _endAz;
    
    // store
    
    startIndex = (int) (_startAz * RAY_LOC_RES);
    endIndex = (int) (_endAz * RAY_LOC_RES + 1);

  }

  for (int ii = startIndex; ii <= endIndex; ii++) {
    _rayLoc[ii].ray = ray;
    _rayLoc[ii].active = true;
    _rayLoc[ii].master = false;
    _rayLoc[ii].startIndex = startIndex;
    _rayLoc[ii].endIndex = endIndex;
  }

  // indicate which ray is the master
  // i.e. it is responsible for ray memory
  
  _rayLoc[midIndex].master = true;

}

/////////////////////////////////////////////////////
// clear previous locations associated with this ray

void HawkEyeGl::_clearRayLoc(int midIndex)
{

  const RayLoc &loc = _rayLoc[midIndex];
  if (loc.active) {
    for (int ii = loc.startIndex; ii <= loc.endIndex; ii++) {
      if (_rayLoc[ii].master && _rayLoc[ii].ray) {
	delete _rayLoc[ii].ray; 
      }
      _rayLoc[ii].ray = NULL;
      _rayLoc[ii].active = false;
      _rayLoc[ii].master = false;
    }
  }
  
}

////////////////////////////////////////////////////////////////
void HawkEyeGl::keyPressEvent(QKeyEvent * e)
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

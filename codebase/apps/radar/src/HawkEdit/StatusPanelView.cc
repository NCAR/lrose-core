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
// StatusPanelView.cc
//
// Status Panel View
//
// Brenda Javornik, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// June 2023
//
///////////////////////////////////////////////////////////////
//
// StatusPanelView manages the display of metadata for the 
// selected (file, sweep, ray, range) = (time, elevation, azimuth, range)
//
///////////////////////////////////////////////////////////////

#include "StatusPanelView.hh"
#include "DisplayField.hh"
#include "FieldListView.hh"
#include "PpiWidget.hh"
#include "RhiWidget.hh"
#include "RhiWindow.hh"
#include "Params.hh"
#include "Reader.hh"
#include "AllocCheck.hh"
#include "BoundaryPointEditor.hh"

#include <string>
#include <cmath>
#include <iostream>
#include <algorithm>    // std::find
#include <Ncxx/H5x.hh>
#include <QActionGroup>
#include <QApplication>
#include <QButtonGroup>
#include <QDialogButtonBox>
#include <QDir>
#include <QFrame>
#include <QFont>
#include <QLabel>
#include <QToolTip>
#include <QMenu>
#include <QMenuBar>
#include <QGroupBox>
#include <QMessageBox>
#include <QFormLayout>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QListWidget>
#include <QListWidgetItem>
#include <QStringListModel>
#include <QRadioButton>
#include <QStatusBar>
#include <QDateTime>
#include <QDateTimeEdit>
#include <QLineEdit>
#include <QErrorMessage>
#include <QFileDialog>
#include <QSlider>
#include <QGraphicsScene>
#include <QGraphicsAnchorLayout>
#include <QGraphicsProxyWidget>

#include <cstdlib>
#include <fstream>
#include <toolsa/toolsa_macros.h>
#include <toolsa/pmu.h>
#include <toolsa/file_io.h>
#include <toolsa/DateTime.hh>
#include <toolsa/LogStream.hh>
#include <dsserver/DsLdataInfo.hh>
#include <radar/RadarComplex.hh>

#include <toolsa/toolsa_macros.h>
#include <toolsa/Path.hh>

#include "CloseEventFilter.hh"
using namespace std;
using namespace H5x;

// Constructor


StatusPanelView::StatusPanelView(QWidget *parent)
{

  _parent = parent;

  // initialize

  _radarLat = -9999.0;
  _radarLon = -9999.0;
  _radarAltKm = -9999.0;

  _altitudeInFeet = false;
  // end from DisplayManager    

  _nGates = 1000;
  _maxRangeKm = 1.0;

  _radarNameRow = 0;
  _dateRow = 1;
  _timeRow = 2; 
  _startingRow = 3;  
  _nrows = _startingRow;  // first three rows are radar name, date, and time

  cerr << "StatusPanelView: _nrows = " << _nrows << "startingRow = " << _startingRow << endl;

  // set up windows

  //_setupWindows();

  // install event filter to catch when the StatusPanel is closed
  CloseEventFilter *closeFilter = new CloseEventFilter(this);
  installEventFilter(closeFilter);

  setAttribute(Qt::WA_DeleteOnClose);

  //connect(this, SIGNAL(readDataFileSignal(vector<string> *)), this, SLOT(inbetweenReadDataFile(vector<string> *)));  
  //connect(this, SIGNAL(fieldSelected(string)), _displayFieldController, SLOT(fieldSelected(string))); 


  //setGeometry(30, 30, 300, 100);

QPalette pal = QPalette();

// set black background
// Qt::black / "#000000" / "black"
//pal.setColor(QPalette::Window, Qt::yellow);

//setAutoFillBackground(true); 
//setPalette(pal);
show();

  _init();
}

// destructor

StatusPanelView::~StatusPanelView()
{

  cerr << "StatusPanelView destructor called " << endl;

}

void StatusPanelView::reset() {
   // delete all the labels and set the number of rows to zero
  _nrows = StatusPanelView::_startingRow;
}


////////////////////////////////////////////
// refresh

void StatusPanelView::_refresh()
{
}

//
// There are set one (when the parameter file changes) 
//       - set font size
// and there are set multiple times:
//   1. when the ray, range (selected azimuth)
//   2. sweep changes (what azimuth to use? )
//   3. when the file changes (reset to 0 azimuth)
// 
void StatusPanelView::setFontSize(int params_label_font_size) {
  QLabel dummy;
  font = dummy.font();
  font2 = dummy.font();
  font6 = dummy.font();
  int fsize = params_label_font_size;
  int fsize2 = params_label_font_size + 2;
  int fsize6 = params_label_font_size + 6;
  font.setPixelSize(fsize);
  font2.setPixelSize(fsize2);
  font6.setPixelSize(fsize6);
}

void StatusPanelView::printHash() {

  HashyIt it;
  for (it=hashy.begin(); it != hashy.end(); ++it) {
    int key = it->first;
    switch (key) {
      case FixedAngleKey:
        cerr << "FixedAngleKey" << endl;
        break;
      case VolumeNumberKey:
        cerr << "VolumeNumberKey" << endl;
        break;
      default:
        cerr << "not found" << endl;

    }
  }
}


void StatusPanelView::clear() {
  // reset values to original 

/*
  // radar and site name
  
  _radarName = new QLabel(_statusPanel);
  string rname(_params->radar_name);
  if (_params->display_site_name) {
    rname += ":";
    rname += _params->site_name;
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

  if (_params->show_status_in_gui.fixed_angle) {
    _fixedAngVal = _createStatusVal("Fixed ang", "-99.99", row++, fsize2);
  } else {
    _fixedAngVal = NULL;
  }
  
  if (_params->show_status_in_gui.volume_number) {
    _volNumVal = _createStatusVal("Volume", "0", row++, fsize);
  } else {
    _volNumVal = NULL;
  }
  
  if (_params->show_status_in_gui.sweep_number) {
    _sweepNumVal = _createStatusVal("Sweep", "0", row++, fsize);
  } else {
    _sweepNumVal = NULL;
  }

  if (_params->show_status_in_gui.n_samples) {
    _nSamplesVal = _createStatusVal("N samp", "0", row++, fsize);
  } else {
    _nSamplesVal = NULL;
  }

  if (_params->show_status_in_gui.n_gates) {
    _nGatesVal = _createStatusVal("N gates", "0", row++, fsize);
  } else {
    _nGatesVal = NULL;
  }

  if (_params->show_status_in_gui.gate_length) {
    _gateSpacingVal = _createStatusVal("Gate len", "0", row++, fsize);
  } else {
    _gateSpacingVal = NULL;
  }
  
  if (_params->show_status_in_gui.pulse_width) {
    _pulseWidthVal = _createStatusVal("Pulse width", "-9999", row++, fsize);
  } else {
    _pulseWidthVal = NULL;
  }

  if (_params->show_status_in_gui.prf_mode) {
    _prfModeVal = _createStatusVal("PRF mode", "Fixed", row++, fsize);
  } else {
    _prfModeVal = NULL;
  }

  if (_params->show_status_in_gui.prf) {
    _prfVal = _createStatusVal("PRF", "-9999", row++, fsize);
  } else {
    _prfVal = NULL;
  }

  if (_params->show_status_in_gui.nyquist) {
    _nyquistVal = _createStatusVal("Nyquist", "-9999", row++, fsize);
  } else {
    _nyquistVal = NULL;
  }

  if (_params->show_status_in_gui.max_range) {
    _maxRangeVal = _createStatusVal("Max range", "-9999", row++, fsize);
  } else {
    _maxRangeVal = NULL;
  }

  if (_params->show_status_in_gui.unambiguous_range) {
    _unambigRangeVal = _createStatusVal("U-A range", "-9999", row++, fsize);
  } else {
    _unambigRangeVal = NULL;
  }

  if (_params->show_status_in_gui.measured_power_h) {
    _powerHVal = _createStatusVal("Power H", "-9999", row++, fsize);
  } else {
    _powerHVal = NULL;
  }

  if (_params->show_status_in_gui.measured_power_v) {
    _powerVVal = _createStatusVal("Power V", "-9999", row++, fsize);
  } else {
    _powerVVal = NULL;
  }

// who holds state?  who holds the _scanNameVal pointer = NULL if not used. 
// the view must hold this
  if (_params->show_status_in_gui.scan_name) {
    _scanNameVal = _createStatusVal("Scan name", "unknown", row++, fsize);
  } else {
    _scanNameVal = NULL;
  }

  if (_params->show_status_in_gui.scan_mode) {
    _sweepModeVal = _createStatusVal("Scan mode", "SUR", row++, fsize);
  } else {
    _sweepModeVal = NULL;
  }

  if (_params->show_status_in_gui.polarization_mode) {
    _polModeVal = _createStatusVal("Pol mode", "Single", row++, fsize);
  } else {
    _polModeVal = NULL;
  }

  if (_params->show_status_in_gui.latitude) {
    _latVal = _createStatusVal("Lat", "-99.999", row++, fsize);
  } else {
    _latVal = NULL;
  }

  if (_params->show_status_in_gui.longitude) {
    _lonVal = _createStatusVal("Lon", "-999.999", row++, fsize);
  } else {
    _lonVal = NULL;
  }

  if (_params->show_status_in_gui.altitude) {
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

  if (_params->show_status_in_gui.altitude_rate) {
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

  if (_params->show_status_in_gui.speed) {
    _speedVal = _createStatusVal("Speed(m/s)", "-999.99", row++, fsize);
  } else {
    _speedVal = NULL;
  }

  if (_params->show_status_in_gui.heading) {
    _headingVal = _createStatusVal("Heading(deg)", "-999.99", row++, fsize);
  } else {
    _headingVal = NULL;
  }

  if (_params->show_status_in_gui.track) {
    _trackVal = _createStatusVal("Track(deg)", "-999.99", row++, fsize);
  } else {
    _trackVal = NULL;
  }

  if (_params->show_status_in_gui.sun_elevation) {
    _sunElVal = _createStatusVal("Sun el (deg)", "-999.999", row++, fsize);
  } else {
    _sunElVal = NULL;
  }

  if (_params->show_status_in_gui.sun_azimuth) {
    _sunAzVal = _createStatusVal("Sun az (deg)", "-999.999", row++, fsize);
  } else {
    _sunAzVal = NULL;
  }

  _georefsApplied = _createStatusVal("Georefs applied?", "T/F", row++, fsize,
    &_georefsAppliedLabel);
  _geoRefRotationVal = _createStatusVal("Georef Rot (deg)", "0.0", row++, fsize,
    &_geoRefRotationLabel);
  _geoRefRollVal = _createStatusVal("Georef Roll (deg)", "0.0", row++, fsize,
    &_geoRefRollLabel);
  _geoRefTiltVal = _createStatusVal("Georef Tilt (deg)", "0.0", row++, fsize,
    &_geoRefTiltLabel);
  _geoRefTrackRelRotationVal = _createStatusVal("Track Rel Rot (deg)", "0.0", row++, fsize,
    &_geoRefTrackRelRotationLabel);
  _geoRefTrackRelTiltVal = _createStatusVal("Track Rel  Tilt (deg)", "0.0", row++, fsize,
    &_geoRefTrackRelTiltLabel);
  _geoRefTrackRelAzVal = _createStatusVal("Track Rel  Az (deg)", "0.0", row++, fsize,
    &_geoRefTrackRelAzLabel);
  _geoRefTrackRelElVal = _createStatusVal("Track Rel  El (deg)", "0.0", row++, fsize,
    &_geoRefTrackRelElLabel);
  _cfacRotationVal = _createStatusVal("Cfac Rot (deg)", "0.0", row++, fsize,
    &_cfacRotationLabel);
  _cfacRollVal = _createStatusVal("Cfac Roll (deg)", "", row++, fsize,
    &_cfacRollLabel);
  _cfacTiltVal = _createStatusVal("Cfac Tilt (deg)", "", row++, fsize,
    &_cfacTiltLabel);
                            
  QLabel *spacerRow = new QLabel("", _statusPanel);
  _statusLayout->addWidget(spacerRow, row, 0);
  _statusLayout->setRowStretch(row, 1);
  row++;

  hideCfacs(); 
*/
}


//////////////////////////////////////////////
// create the status panel

void StatusPanelView::_init()
{
 
  Qt::Alignment alignLeft(Qt::AlignLeft);
  Qt::Alignment alignRight(Qt::AlignRight);
  Qt::Alignment alignCenter(Qt::AlignCenter);
  Qt::Alignment alignTop(Qt::AlignTop);

  // status panel - rows of label value pairs
  
  //_statusPanel = new QGroupBox(_parent);
  _statusLayout = new QGridLayout(this);
  setLayout(_statusLayout);
  //_statusLayout->setVerticalSpacing(5);

  //QLabel *hello = new QLabel();
  //hello->setText("hello");
  //_statusLayout->addWidget(hello);

  //int row = 0;

  _elevVal = _createStatusVal("Elev", "-99.99", _fsize2);
  _azVal = _createStatusVal("Az", "-999.99", _fsize2);

/*
struct S {
  string label; // =   "Volume",
  string emptyValue; // =  "0",
  string format;
  QLabel *value; 
  int fontSize;
};
*/

  hashy.reserve(LastKey);

  // what to do about float format?
  _metaDataS[FixedAngleKey] =   {"Fixed ang", "-99.99", "f6.2???", _fsize2, NULL, NULL};
  _metaDataS[VolumeNumberKey] = {"Volume", "0", "%d", _fsize, NULL, NULL};  
  _metaDataS[SweepNumberKey] =  {"Sweep", "0", "%d", _fsize, NULL, NULL};  

  _metaDataS[NSamplesKey] =   {"N samp", "0", "%d", _fsize, NULL, NULL};
  _metaDataS[NGatesKey] = {"N gates", "0", "%d", _fsize, NULL, NULL};  
  _metaDataS[GateSpacingKey] =  {"Gate len", "0", "%d", _fsize, NULL, NULL};  
  _metaDataS[PulseWidthKey] =   {"Pulse width", "-9999", "f6.2???", _fsize, NULL, NULL};
  _metaDataS[PrfModeKey] = {"PRF mode", "fixed", "%d", _fsize, NULL, NULL};  
  _metaDataS[PrfKey] =  {"PRF", "-9999", "%d", _fsize, NULL, NULL};  
  _metaDataS[NyquistKey] =   {"Nyquist", "-9999", "f6.2???", _fsize, NULL, NULL};
  _metaDataS[MaxRangeKey] = {"Max range", "-9999", "%d", _fsize, NULL, NULL};  
  _metaDataS[UnambiguousRangeKey] =  {"U-A range", "-9999", "%d", _fsize, NULL, NULL};  
  _metaDataS[PowerHKey] =   {"Power H", "-9999", "f6.2???", _fsize, NULL, NULL};
  _metaDataS[PowerVKey] = {"Power V", "-9999", "%d", _fsize, NULL, NULL};  
  _metaDataS[ScanNameKey] =  {"Scan name", "unknown", "%s", _fsize, NULL, NULL};  

  _metaDataS[SweepModeKey] =   {"Scan mode", "SUR", "f6.2???", _fsize, NULL, NULL};
  _metaDataS[PolarizationModeKey] = {"Pol mode", "Single", "%d", _fsize, NULL, NULL};  
  _metaDataS[LatitudeKey] =  {"Lat", "-999.999", "%d", _fsize, NULL, NULL};  
  _metaDataS[LongitudeKey] =   {"Lon", "-999.999", "f6.2???", _fsize, NULL, NULL};
  _metaDataS[AltitudeInFeetKey] = {"Alt(kft)", "-999.999", "%d", _fsize, NULL, NULL};  
  _metaDataS[AltitudeInKmKey] =  {"Alt(km)", "-999.999", "%d", _fsize, NULL, NULL};  

  _metaDataS[AltitudeRateFtsKey] =   {"AltRate(ft/s)", "-999.999", "f6.2???", _fsize, NULL, NULL};
  _metaDataS[AltitudeRateMsKey] = {"AltRate(m/s)", "-999.999", "%d", _fsize, NULL, NULL};  
  _metaDataS[SpeedKey] =  {"Speed(m/s)", "-999.999", "%d", _fsize, NULL, NULL};  
  _metaDataS[HeadingKey] =   {"Heading(deg)", "-999.999", "f6.2???", _fsize, NULL, NULL};
  _metaDataS[TrackKey] = {"Track(deg)", "-999.999", "%d", _fsize, NULL, NULL};  
  _metaDataS[SunElevationKey] =  {"Sun el (deg)", "-999.999", "%d", _fsize, NULL, NULL};  
  _metaDataS[SunAzimuthKey] =   {"Sun az (deg)", "-999.999", "f6.2???", _fsize, NULL, NULL};


  _metaDataS[GeoRefsAppliedKey] = {"Georefs applied?", "T/F", "%s", _fsize, NULL, NULL};  
  _metaDataS[GeoRefRotationKey] =  {"Georef Rot (deg)", "0.0", "%d", _fsize, NULL, NULL};
  _metaDataS[GeoRefRollKey] =  {"Georef Roll (deg)", "0.0", "%d", _fsize, NULL, NULL};            
  _metaDataS[GeoRefTiltKey] =   {"Georef Tilt (deg)", "0.0", "f6.2???", _fsize, NULL, NULL};

  _metaDataS[GeoRefTrackRelRotationKey] = {"Track Rel Rot (deg)", "0.0", "%d", _fsize, NULL, NULL};  
  _metaDataS[GeoRefTrackRelTiltKey] =  {"Track Rel  Tilt (deg)", "0.0", "%d", _fsize, NULL, NULL};  
  _metaDataS[GeoRefTrackRelAzimuthKey] =   {"Track Rel  Az (deg)", "0.0", "f6.2???", _fsize, NULL, NULL};
  _metaDataS[GeoRefTrackRelElevationKey] = {"Track Rel  El (deg)", "0.0", "%d", _fsize, NULL, NULL};  

  _metaDataS[CfacRotationKey] =  {"Cfac Rot (deg)", "0.0", "%d", _fsize, NULL, NULL};  
  _metaDataS[CfacRollKey] =   {"Cfac Roll (deg)", "0.0", "f6.2???", _fsize, NULL, NULL};
  _metaDataS[CfacTiltKey] = {"Cfac Tilt (deg)", "0.0", "%d", _fsize, NULL, NULL};  
        

}

  // radar and site name
  

void StatusPanelView::setRadarName(string radarName, string siteName) {
  _radarName = new QLabel(this);
  string rname(radarName);
  if (siteName.length() > 0) {
    rname.append(":");
    rname.append(siteName);
  }
  _radarName->setText(rname.c_str());
  _radarName->setFont(font6);

  // always put radar name at row zero
  //Qt::Alignment alignCenter(Qt::AlignCenter); 
  _statusLayout->addWidget(_radarName, _radarNameRow, 0, 1, 2, Qt::AlignCenter);
}

void StatusPanelView::createDateTime() {
  // date and time

  _dateVal = new QLabel("9999/99/99", this);
  _dateVal->setFont(font2);
  _statusLayout->addWidget(_dateVal, _dateRow, 0, 1, 2, Qt::AlignCenter); // , row, 0, 1, 2, alignCenter);
  

  _timeVal = new QLabel("99:99:99.999", this);
  _timeVal->setFont(font2);
  _statusLayout->addWidget(_timeVal, _timeRow, 0, 1, 2, Qt::AlignCenter); // , row, 0, 1, 2, alignCenter);
  
}

void StatusPanelView::createGeoreferenceLabels() {

  for (int key = GeoRefsAppliedKey; key <= LastKey; key++) {
    createOnly(key);  
  }
  hideCfacs();
}

/*

  // other labels.  Note that we set the minimum size of the column
  // containing the right hand labels in timerEvent() to prevent the
  // wiggling we were seeing in certain circumstances.  For this to work,
  // the default values for these fields must represent the maximum digits
  // posible for each field.

*/

/*
// Q_OBJECT does not support templates (unless the template class inherits from the Q_OBJECT class)
// s is the value
// template<typename T>
*/
    void StatusPanelView::setInt(int s, QLabel *label) { // , string format) {
      if (label != NULL) {

        int value = -9999;

        if (abs(s) < 9999) {
          value = s;
        }
        label->setText(QString("%1").arg(value));
      }
    }
    
    void StatusPanelView::setDouble(double s, QLabel *label, int fieldWidth, int precision) {
      if (label != NULL) {

        double value = -9999;

        if (abs(s) < 9999) {
          value = s;
        }
        label->setText(QString("%1").arg(value, fieldWidth, 'f', precision));
      }
    }

     void StatusPanelView::setString(string s, QLabel *label) { // , string format) {
      if (label != NULL) {
        label->setText(QString(s.c_str()));
      }
    }   



/* s is the value
template<typename T>
    QLabel *StatusPanelView::create(T s, string textLabel, string format) {
      if (label == NULL) {
        _nrows += 1;
        label = _createStatusVal(textLabel, s, _nrows, _fsize2);
      } 
      return label;
    }    
*/
// Q: How do we know which row in the display? only need the row when creating.
/*
// to use (in Controller???): 
//   set<double>(s, "Fixed ang", "%d", row??)
void StatusPanelView::setFixedAngleDeg(double fixedAngleDeg) {
  setDouble(fixedAngleDeg, _fixedAngVal, 7, 2);
}
  
void StatusPanelView::createFixedAngleDeg() {
    _fixedAngVal = _createStatusVal("Fixed ang", "-99.99", _fsize2);
}

void StatusPanelView::setVolumeNumber(int volumeNumber) {
  setInt(volumeNumber, _volNumVal);// , "%d");
}

void StatusPanelView::createVolumeNumber() {
  _volNumVal = _createStatusVal("Volume", "0", _fsize);
}

void StatusPanelView::setSweepNum(int sweepNumber) {
  setInt(sweepNumber, _sweepNumVal); //  "%d");
}

void StatusPanelView::createSweepNumber() {
  // create the label 
  _sweepNumVal = _createStatusVal("Sweep", "0", _fsize);
} 

*/ 
//                <Key,         Hash,       KeyEqual,    Allocator>::find
//                <data_type> 
//std::unordered_map<int, S> hashy;
//hashy.insert(obj);
//hashy.find("one"); // returns an iterator ...

/*
struct S {
  string label; // =   "Volume",
  string emptyValue; // =  "0",
  string format;
  QLabel *value; 
  int fontSize;
};
*/
void StatusPanelView::createOnly(int key) {
  S *s = &_metaDataS[key];
  s->value = _createStatusVal(s->label, s->emptyValue, s->fontSize, &(s->guiLeftLabel));
}

void StatusPanelView::create(int key) {
  createOnly(key);
  S *s = &_metaDataS[key];
  //s->value = _createStatusVal(s->label, s->emptyValue, s->fontSize);
  hashy[key] = s; 
  // hashy[key] now has the pointer to the QLabel for the sweep number
}

// _view->set(SweepNumKey, ray->getSweepNumber());
void StatusPanelView::set(int key, int value) {
  S *s = hashy[key];
  setInt(value, s->value); // , "%d");  // set for int always uses "%d" format.
  //setInt(value, hashy[key], hashyFormat[key]);
}

void StatusPanelView::set(int key, double value) {
  S *s = hashy[key];
  setDouble(value, s->value, 6, 2);
}

void StatusPanelView::set(int key, string value) {
  S *s = hashy[key];
  setString(value, s->value);
}

// Why keep the georefs separate? why not just put them into the hash table too?
// because the georefs are hidden sometimes if the data are NOT airborne.
void StatusPanelView::setNoHash(int key, int value) {
  setInt(value, _metaDataS[key].value);
}

void StatusPanelView::setNoHash(int key, double value) {
  setDouble(value, _metaDataS[key].value, 6, 2); // _metaDataS[key].fieldWidth, _metatDataS[key].precision);
  // (double s, QLabel *label, int fieldWidth, int precision) 
}

void StatusPanelView::setNoHash(int key, string value) {
  setString(value, _metaDataS[key].value);
}

/*
void StatusPanelView::set(int key, double value) {
  S *s = hashy[key];
  setDouble(value, s->value); // , "%d");  // set for int always uses "%d" format.
  //setInt(value, hashy[key], hashyFormat[key]);
}
*/

// each of theses should be a class/factory object that are placed in a list
// the class must associate the ray varable with the panel variable.
// Use parallel lists / arrays.
// clear and initialize when new parameter file is read.
// stuff the label pointer into a list
// stuff the values from the ray into a list
// create: stuff the (strings, value as a string, font size) into a list
//    set: stuff the ray values into a list
// loop through the lists in parallel and set the values.
// loop through the list and create the labels.
// so much less code to maintain.

/*
void StatusPanelView::setNSamples(int nSamples) {
  setInt(nSamples, _nSamplesVal, "%d");  
}

void StatusPanelView::updateStatusPanel(vector<double>


void StatusPanelView::createNSamples() {
    _nSamplesVal = _createStatusVal("N samp", "0", _fsize);
}

void StatusPanelView::createNGates() {
    _nGatesVal = _createStatusVal("N gates", "0", _fsize);
}

void StatusPanelView::createGateSpacing() {
    _gateSpacingVal = _createStatusVal("Gate len", "0", row++, fsize);
}
  
void StatusPanelView::createPulseWidth() {  
    _pulseWidthVal = _createStatusVal("Pulse width", "-9999", row++, fsize);
} 

void StatusPanelView::createPrfMode() {
    _prfModeVal = _createStatusVal("PRF mode", "Fixed", row++, fsize);
}

void StatusPanelView::createPrf() {
    _prfVal = _createStatusVal("PRF", "-9999", row++, fsize);
}

void StatusPanelView::createNyquist() {
    _nyquistVal = _createStatusVal("Nyquist", "-9999", row++, fsize);
}

void StatusPanelView::createMaxRange() {
    _maxRangeVal = _createStatusVal("Max range", "-9999", row++, fsize);
}

void StatusPanelView::createUnambiguousRange() {
    _unambigRangeVal = _createStatusVal("U-A range", "-9999", row++, fsize);
}

void StatusPanelView::createPowerH() {
    _powerHVal = _createStatusVal("Power H", "-9999", row++, fsize);
}

void StatusPanelView::createPowerV() {
    _powerVVal = _createStatusVal("Power V", "-9999", row++, fsize);
}

void StatusPanelView::createScanName() {
    _scanNameVal = _createStatusVal("Scan name", "unknown", row++, fsize);
}


  if (_params->show_status_in_gui.scan_mode) {
    _sweepModeVal = _createStatusVal("Scan mode", "SUR", row++, fsize);
  } else {
    _sweepModeVal = NULL;
  }

  if (_params->show_status_in_gui.polarization_mode) {
    _polModeVal = _createStatusVal("Pol mode", "Single", row++, fsize);
  } else {
    _polModeVal = NULL;
  }

  if (_params->show_status_in_gui.latitude) {
    _latVal = _createStatusVal("Lat", "-99.999", row++, fsize);
  } else {
    _latVal = NULL;
  }

  if (_params->show_status_in_gui.longitude) {
    _lonVal = _createStatusVal("Lon", "-999.999", row++, fsize);
  } else {
    _lonVal = NULL;
  }

  if (_params->show_status_in_gui.altitude) {
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

  if (_params->show_status_in_gui.altitude_rate) {
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

  if (_params->show_status_in_gui.speed) {
    _speedVal = _createStatusVal("Speed(m/s)", "-999.99", _fsize);
  } else {
    _speedVal = NULL;
  }

  if (_params->show_status_in_gui.heading) {
    _headingVal = _createStatusVal("Heading(deg)", "-999.99", _fsize);
  } else {
    _headingVal = NULL;
  }

  if (_params->show_status_in_gui.track) {
    _trackVal = _createStatusVal("Track(deg)", "-999.99", _fsize);
  } else {
    _trackVal = NULL;
  }

  if (_params->show_status_in_gui.sun_elevation) {
    _sunElVal = _createStatusVal("Sun el (deg)", "-999.999", _fsize);
  } else {
    _sunElVal = NULL;
  }

  if (_params->show_status_in_gui.sun_azimuth) {
    _sunAzVal = _createStatusVal("Sun az (deg)", "-999.999", _fsize);
  } else {
    _sunAzVal = NULL;
  }

  _georefsApplied = _createStatusVal("Georefs applied?", "T/F", row++, fsize,
    &_georefsAppliedLabel);
  _geoRefRotationVal = _createStatusVal("Georef Rot (deg)", "0.0", row++, fsize,
    &_geoRefRotationLabel);
  _geoRefRollVal = _createStatusVal("Georef Roll (deg)", "0.0", row++, fsize,
    &_geoRefRollLabel);
  _geoRefTiltVal = _createStatusVal("Georef Tilt (deg)", "0.0", row++, fsize,
    &_geoRefTiltLabel);
  _geoRefTrackRelRotationVal = _createStatusVal("Track Rel Rot (deg)", "0.0", row++, fsize,
    &_geoRefTrackRelRotationLabel);
  _geoRefTrackRelTiltVal = _createStatusVal("Track Rel  Tilt (deg)", "0.0", row++, fsize,
    &_geoRefTrackRelTiltLabel);
  _geoRefTrackRelAzVal = _createStatusVal("Track Rel  Az (deg)", "0.0", row++, fsize,
    &_geoRefTrackRelAzLabel);
  _geoRefTrackRelElVal = _createStatusVal("Track Rel  El (deg)", "0.0", row++, fsize,
    &_geoRefTrackRelElLabel);
  _cfacRotationVal = _createStatusVal("Cfac Rot (deg)", "0.0", row++, fsize,
    &_cfacRotationLabel);
  _cfacRollVal = _createStatusVal("Cfac Roll (deg)", "", row++, fsize,
    &_cfacRollLabel);
  _cfacTiltVal = _createStatusVal("Cfac Tilt (deg)", "", row++, fsize,
    &_cfacTiltLabel);
                            
  QLabel *spacerRow = new QLabel("", _statusPanel);
  _statusLayout->addWidget(spacerRow, row, 0);
  _statusLayout->setRowStretch(row, 1);
  row++;

  hideCfacs(); 

}
*/

//////////////////////////////////////////////
/* make a new label with right justification

QLabel *StatusPanelView::_newLabelRight(const string &text)
{
  QLabel *label = new QLabel;
  label->setText("-----");
  label->setAlignment(Qt::AlignRight);
  return label;
}
*/

//////////////////////////////////////////////////
// create a row in the status panel

QLabel *StatusPanelView::_createStatusVal(const string &leftLabel,
                                         const string &rightLabel,
                                         int fontSize,
                                         QLabel **guiLeftLabel)  
{
  _nrows += 1;
  int row = _nrows;
  QLabel *left = new QLabel(this);
  left->setText(leftLabel.c_str());
  Qt::Alignment alignRight(Qt::AlignRight);
  _statusLayout->addWidget(left, row, 0, alignRight);
  if (guiLeftLabel != NULL) {
    *guiLeftLabel = left;
  }

  QLabel *right = new QLabel(this);
  right->setText(rightLabel.c_str());
  Qt::Alignment alignCenter(Qt::AlignCenter);
  _statusLayout->addWidget(right, row, 1, alignRight);

  if (fontSize > 0) {
    QFont font = left->font();
    font.setPixelSize(fontSize);
    left->setFont(font);
    right->setFont(font);
  }

  //_valsRight.push_back(right);

  return right;
}

//////////////////////////////////////////////////
/* create a label row in a dialog

QLabel *StatusPanelView::_addLabelRow(QWidget *parent,
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

*/


//////////////////////////////////////////////
// update the status panel

void StatusPanelView::setDateTime(int year, int month, int day,
  int hour, int minutes, int seconds,  int nanoSeconds) {

  // set time etc

  //char text[1024];

  //sprintf(text, "%.4d/%.2d/%.2d",
  //        rayTime.getYear(), rayTime.getMonth(), rayTime.getDay());

  QDate rayDate(year, month, day);
  _dateVal->setText(rayDate.toString("yyyy/MM/dd"));

  //sprintf(text, "%.2d:%.2d:%.2d.%.3d",
  //        rayTime.getHour(), rayTime.getMin(), rayTime.getSec(),
  //        ((int) nanoSeconds / 1000000));

  QTime rayTime(hour, minutes, seconds, (int) nanoSeconds / 1000000);
  _timeVal->setText(rayTime.toString("hh:mm:ss.zzz"));

}

/*
void StatusPanelView::updateStatusPanel(
    string volumeNumber,
    string sweepNumber,
    string fixedAngleDeg,
    string elevationDeg,
    string azimuthDeg,
    string nSamples,
    string nGates,
    string gateSpacingKm,
    string pulseWidthUsec,
    string nyquistMps)
  //const RadxRay *ray)
{

  // set time etc

  char text[1024];
  
  QString prev_radar_name = _radarName->text();
  
  string rname(_platform.getInstrumentName());
  if (_params->override_radar_name) rname = _params->radar_name;
  if (_params->display_site_name) {
    rname += ":";
    if (_params->override_site_name) {
      rname += _params->site_name;
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
  */


  /*
  if (_fixedAngVal) {  
    _setText(text, "%6.2f", ray->getFixedAngleDeg());
    _fixedAngVal->setText(text);
  }
  */
  void StatusPanelView::updateAzEl(double azDeg, double elDeg) {
    if (abs(azDeg) < 1000) {
      setDouble(azDeg, _azVal, 6, 2);
    }
    if (abs(elDeg) < 1000) {
      setDouble(elDeg, _elevVal, 6, 2);
    }

    clear();
  }
  /*
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
    _setText(text, "%.2f", ray->getPulseWidthUsec());
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
      _setText(text, "%.3f", _platform.getAltitudeKm() / 0.3048);
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

  // if airborne data ...
  if (ray->getSweepMode() == Radx::SWEEP_MODE_ELEVATION_SURVEILLANCE) {
    
    _georefsApplied->show();
    _geoRefRotationVal->show();
    _geoRefRollVal->show();
    _geoRefTiltVal->show();
    _geoRefTrackRelRotationVal->show(); 
    _geoRefTrackRelTiltVal->show(); 
    _geoRefTrackRelAzVal->show(); 
    _geoRefTrackRelElVal->show(); 
    _cfacRotationVal->show();
    _cfacRollVal->show();
    _cfacTiltVal->show();     
    //_applyCfacToggle->show();

    _georefsAppliedLabel->show(); 
    _geoRefRotationLabel->show(); 
    _geoRefRollLabel->show(); 
    _geoRefTiltLabel->show();
    _geoRefTrackRelRotationLabel->show(); 
    _geoRefTrackRelTiltLabel->show(); 
    _geoRefTrackRelAzLabel->show(); 
    _geoRefTrackRelElLabel->show(); 
    _cfacRotationLabel->show(); 
    _cfacRollLabel->show(); 
    _cfacTiltLabel->show();     

    const RadxGeoref *georef = ray->getGeoreference();
    if (georef != NULL) {

      if (ray->getGeorefApplied()) {
        _georefsApplied->setText("true");       
      } else {
        _georefsApplied->setText("false");          
      }

      _setText(text, "%.3f", georef->getRotation());  
      _geoRefRotationVal->setText(text); 
      _setText(text, "%.3f", georef->getRoll());  
      _geoRefRollVal->setText(text); 
      _setText(text, "%.3f", georef->getTilt());  
      _geoRefTiltVal->setText(text);

      _setText(text, "%.3f", georef->getTrackRelRot());  
      _geoRefTrackRelRotationVal->setText(text); 
      _setText(text, "%.3f", georef->getTrackRelTilt());  
      _geoRefTrackRelTiltVal->setText(text);
      _setText(text, "%.3f", georef->getTrackRelAz());  
      _geoRefTrackRelAzVal->setText(text); 
      _setText(text, "%.3f", georef->getTrackRelEl());  
      _geoRefTrackRelElVal->setText(text); 

      double rollCorr = 0.0;
      double rotCorr = 0.0;
      double tiltCorr = 0.0;
      DataModel *dataModel = DataModel::Instance();
      dataModel->getCfactors(&rollCorr, &rotCorr, &tiltCorr);
      _setText(text, "%.3f", rollCorr);
      _cfacRollVal->setText(text);
      _setText(text, "%.3f", rotCorr);
      _cfacRotationVal->setText(text);   
      _setText(text, "%.3f", tiltCorr);    
      _cfacTiltVal->setText(text);

    }
  } else {
    hideCfacs();
  }

 
}
*/
void StatusPanelView::hideCfacs() {

  for (int key = GeoRefsAppliedKey; key <= LastKey; key++) {
    //S s = _metaDataS[key];
    QLabel *value = _metaDataS[key].value;
    value->hide();
    QLabel *leftLabel = _metaDataS[key].guiLeftLabel;
    leftLabel->hide();
  }
}
/*
  _georefsApplied->hide(); 
  _geoRefRotationVal->hide(); 
  _geoRefRollVal->hide(); 
  _geoRefTiltVal->hide();   
  _geoRefTrackRelRotationVal->hide(); 
  _geoRefTrackRelAzVal->hide(); 
  _geoRefTrackRelTiltVal->hide();  
  _geoRefTrackRelElVal->hide();  
  _cfacRotationVal->hide(); 
  _cfacRollVal->hide(); 
  _cfacTiltVal->hide(); 
  _georefsAppliedLabel->hide(); 
  _geoRefRotationLabel->hide(); 
  _geoRefRollLabel->hide(); 
  _geoRefTiltLabel->hide(); 
  _geoRefTrackRelRotationLabel->hide(); 
  _geoRefTrackRelAzLabel->hide(); 
  _geoRefTrackRelTiltLabel->hide();   
  _geoRefTrackRelElLabel->hide(); 
  _cfacRotationLabel->hide(); 
  _cfacRollLabel->hide(); 
  _cfacTiltLabel->hide(); 

  //_applyCfacToggle->hide();  
}
*/
void StatusPanelView::showCfacs() {

  for (int key = GeoRefsAppliedKey; key <= LastKey; key++) {
    //S s = _metaDataS[key];
    QLabel *value = _metaDataS[key].value;
    value->show();
    QLabel *leftLabel = _metaDataS[key].guiLeftLabel;
    leftLabel->show();
  }
}
/*
      _georefsApplied->show();
      _geoRefRotationVal->show();
      _geoRefRollVal->show();
      _geoRefTiltVal->show();
      _geoRefTrackRelRotationVal->show(); 
      _geoRefTrackRelTiltVal->show(); 
      _geoRefTrackRelAzVal->show(); 
      _geoRefTrackRelElVal->show(); 
      _cfacRotationVal->show();
      _cfacRollVal->show();
      _cfacTiltVal->show();     

      _georefsAppliedLabel->show(); 
      _geoRefRotationLabel->show(); 
      _geoRefRollLabel->show(); 
      _geoRefTiltLabel->show();
      _geoRefTrackRelRotationLabel->show(); 
      _geoRefTrackRelTiltLabel->show(); 
      _geoRefTrackRelAzLabel->show(); 
      _geoRefTrackRelElLabel->show(); 
      _cfacRotationLabel->show(); 
      _cfacRollLabel->show(); 
      _cfacTiltLabel->show();   
}
*/

///////////////////////////////////////////
/* set text for GUI panels

void StatusPanelView::_setText(char *text,
                              const char *format,
                              int val)
{
  if (abs(val) < 9999) {
    sprintf(text, format, val);
  } else {
    sprintf(text, format, -9999);
  }
}

void StatusPanelView::_setText(char *text,
                              const char *format,
                              double val)
{
  if (fabs(val) < 9999) {
    sprintf(text, format, val);
  } else {
    sprintf(text, format, -9999.0);
  }
}
*/
////////////////////////////////////////////////////////////////
/*
double StatusPanelView::_getInstHtKm(const RadxRay *ray)

{
  double instHtKm = _platform.getAltitudeKm();
  if (ray->getGeoreference() != NULL) {
    instHtKm = ray->getGeoreference()->getAltitudeKmMsl();
  }
  return instHtKm;
}
*/
/////////////////////////////////////////////////////////////////////  
// slots


void StatusPanelView::closeEvent(QEvent *event)
{
 
    event->accept();
    //emit close();

}



